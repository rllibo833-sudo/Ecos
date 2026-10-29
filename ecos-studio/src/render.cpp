#include "ecos_studio/render.hpp"
#include "ecos_studio/platform.hpp"
#include "ecos_studio/camera.hpp"
#include "ecos_studio/engine_loop.hpp"
#include "ecos_studio/world_content_runtime.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <fstream>
#include <system_error>
#include <string>
#include <vector>
#include <unordered_map>

namespace ecos::studio {
namespace {
struct ScreenVertex { float x{}, y{}, z{}; };
struct Vec3f { float x{}, y{}, z{}; };
Vec3f subtract(Vec3f a, Vec3f b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
Vec3f cross(Vec3f a, Vec3f b) {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
float dot(Vec3f a, Vec3f b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
float length(Vec3f v) { return std::sqrt(dot(v,v)); }
}

Result<std::string> render_mesh_to_ppm(
    const MeshData& mesh, const Matrix4& world_transform,
    std::uint32_t width, std::uint32_t height) {
    MeshData transformed = mesh;
    for (const float value : world_transform.values) {
        if (!std::isfinite(value)) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "World transform contains non-finite matrix values"});
        }
    }
    for (auto& position : transformed.positions) {
        const float x = position.x;
        const float y = position.y;
        const float z = position.z;
        position = {
            world_transform.values[0] * x + world_transform.values[1] * y +
                world_transform.values[2] * z + world_transform.values[3],
            world_transform.values[4] * x + world_transform.values[5] * y +
                world_transform.values[6] * z + world_transform.values[7],
            world_transform.values[8] * x + world_transform.values[9] * y +
                world_transform.values[10] * z + world_transform.values[11]
        };
    }
    return render_mesh_to_ppm(transformed, width, height);
}

Result<std::string> render_scene_to_ppm(
    const std::vector<RenderMeshInstance>& instances,
    std::uint32_t width, std::uint32_t height) {
    if (instances.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "Scene renderer requires at least one mesh instance"});
    }

    MeshData combined;
    constexpr std::size_t max_scene_vertices = 4U * 1024U * 1024U;
    constexpr std::size_t max_scene_indices = 12U * 1024U * 1024U;
    for (const auto& instance : instances) {
        if (instance.mesh == nullptr) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Scene contains a null mesh instance"});
        }
        const auto& mesh = *instance.mesh;
        if (mesh.positions.empty() || mesh.indices.empty() || mesh.indices.size() % 3 != 0) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Every scene mesh requires vertices and a triangle index buffer"});
        }
        if (mesh.positions.size() > max_scene_vertices - combined.positions.size() ||
            mesh.indices.size() > max_scene_indices - combined.indices.size() ||
            combined.positions.size() > std::numeric_limits<std::uint32_t>::max() - mesh.positions.size()) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Scene geometry exceeds renderer limits"});
        }
        for (const auto index : mesh.indices) {
            if (index >= mesh.positions.size()) {
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "Scene mesh index references a missing vertex"});
            }
        }
        for (const float value : instance.world_transform.values) {
            if (!std::isfinite(value)) {
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "Scene mesh transform contains non-finite matrix values"});
            }
        }

        const auto vertex_offset = static_cast<std::uint32_t>(combined.positions.size());
        for (const auto& position : mesh.positions) {
            const float x = position.x, y = position.y, z = position.z;
            const Vec3 transformed{
                instance.world_transform.values[0] * x + instance.world_transform.values[1] * y +
                    instance.world_transform.values[2] * z + instance.world_transform.values[3],
                instance.world_transform.values[4] * x + instance.world_transform.values[5] * y +
                    instance.world_transform.values[6] * z + instance.world_transform.values[7],
                instance.world_transform.values[8] * x + instance.world_transform.values[9] * y +
                    instance.world_transform.values[10] * z + instance.world_transform.values[11]
            };
            if (!std::isfinite(transformed.x) || !std::isfinite(transformed.y) ||
                !std::isfinite(transformed.z)) {
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "Scene transform produced non-finite vertex coordinates"});
            }
            combined.positions.push_back(transformed);
        }
        for (const auto index : mesh.indices) {
            combined.indices.push_back(vertex_offset + index);
        }
    }
    return render_mesh_to_ppm(combined, width, height);
}



Result<std::string> render_scene_to_ppm(
    const std::vector<RenderMeshInstance>& instances, const Matrix4& view_projection,
    std::uint32_t width, std::uint32_t height) {
    constexpr std::uint32_t max_dimension = 4096;
    if (width == 0 || height == 0 || width > max_dimension || height > max_dimension)
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "Render dimensions must be between 1 and 4096 pixels"});
    if (instances.empty())
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "Camera renderer requires at least one mesh instance"});
    for (const float v : view_projection.values) if (!std::isfinite(v))
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "View-projection matrix contains non-finite values"});

    struct ClipVertex { float x{}, y{}, z{}, w{}; Vec3 world{}; };
    struct RasterVertex { float x{}, y{}, z{}; };
    const auto interpolate = [](const ClipVertex& a, const ClipVertex& b, float t) {
        return ClipVertex{
            a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t,
            a.w+(b.w-a.w)*t,
            {a.world.x+(b.world.x-a.world.x)*t,
             a.world.y+(b.world.y-a.world.y)*t,
             a.world.z+(b.world.z-a.world.z)*t}};
    };
    // OpenGL-style homogeneous clip volume: -w <= x,y,z <= w.
    const auto clip_polygon = [&interpolate](std::vector<ClipVertex> polygon) {
        const std::array<float(*)(const ClipVertex&), 6> planes{
            +[](const ClipVertex& v){ return v.x+v.w; },
            +[](const ClipVertex& v){ return v.w-v.x; },
            +[](const ClipVertex& v){ return v.y+v.w; },
            +[](const ClipVertex& v){ return v.w-v.y; },
            +[](const ClipVertex& v){ return v.z+v.w; },
            +[](const ClipVertex& v){ return v.w-v.z; }};
        for (const auto& distance : planes) {
            if (polygon.empty()) break;
            std::vector<ClipVertex> output;
            output.reserve(polygon.size()+1U);
            auto previous = polygon.back();
            float previous_distance = distance(previous);
            bool previous_inside = previous_distance >= 0.0F;
            for (const auto& current : polygon) {
                const float current_distance = distance(current);
                const bool current_inside = current_distance >= 0.0F;
                if (current_inside != previous_inside) {
                    const float denominator = previous_distance-current_distance;
                    if (std::abs(denominator) > 1.0e-20F) {
                        const float t = std::clamp(previous_distance/denominator, 0.0F, 1.0F);
                        output.push_back(interpolate(previous, current, t));
                    }
                }
                if (current_inside) output.push_back(current);
                previous = current;
                previous_distance = current_distance;
                previous_inside = current_inside;
            }
            polygon = std::move(output);
        }
        return polygon;
    };

    const auto pixel_count = static_cast<std::size_t>(width)*height;
    std::vector<std::uint8_t> pixels(pixel_count*3U);
    std::vector<float> depth(pixel_count, std::numeric_limits<float>::infinity());
    for (std::size_t i=0; i<pixel_count; ++i) {
        pixels[i*3]=18; pixels[i*3+1]=22; pixels[i*3+2]=30;
    }
    const Vec3f light_raw{-0.35F,0.65F,0.68F};
    const float light_len=length(light_raw);
    const Vec3f light{light_raw.x/light_len,light_raw.y/light_len,light_raw.z/light_len};

    for (const auto& instance : instances) {
        if (!instance.mesh) return Result<std::string>::failure(
            {ErrorCode::invalid_argument, "Scene contains a null mesh instance"});
        for (const float v : instance.world_transform.values) if (!std::isfinite(v))
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Scene mesh transform contains non-finite matrix values"});
        const auto valid_color = [](float value) { return std::isfinite(value) && value >= 0.0F && value <= 1.0F; };
        if (!valid_color(instance.diffuse_color.x) || !valid_color(instance.diffuse_color.y) || !valid_color(instance.diffuse_color.z))
            return Result<std::string>::failure({ErrorCode::invalid_argument, "Render material diffuse color components must be finite and between 0 and 1"});
        const auto& mesh=*instance.mesh;
        if (mesh.positions.empty() || mesh.indices.empty() || mesh.indices.size()%3U!=0)
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Every scene mesh requires vertices and a triangle index buffer"});
        std::vector<ClipVertex> vertices;
        vertices.reserve(mesh.positions.size());
        for (const auto& p : mesh.positions) {
            if (!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z))
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "Mesh contains non-finite vertex coordinates"});
            const Vec3 world{
                instance.world_transform.values[0]*p.x+instance.world_transform.values[1]*p.y+instance.world_transform.values[2]*p.z+instance.world_transform.values[3],
                instance.world_transform.values[4]*p.x+instance.world_transform.values[5]*p.y+instance.world_transform.values[6]*p.z+instance.world_transform.values[7],
                instance.world_transform.values[8]*p.x+instance.world_transform.values[9]*p.y+instance.world_transform.values[10]*p.z+instance.world_transform.values[11]};
            if (!std::isfinite(world.x)||!std::isfinite(world.y)||!std::isfinite(world.z))
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "World transform produced non-finite vertex coordinates"});
            const auto clip=transform(view_projection,{world.x,world.y,world.z,1.0F});
            if (!std::isfinite(clip.x)||!std::isfinite(clip.y)||!std::isfinite(clip.z)||!std::isfinite(clip.w))
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "View-projection transform produced non-finite clip coordinates"});
            vertices.push_back({clip.x,clip.y,clip.z,clip.w,world});
        }

        for (std::size_t tri=0; tri<mesh.indices.size(); tri+=3U) {
            const auto ia=mesh.indices[tri], ib=mesh.indices[tri+1], ic=mesh.indices[tri+2];
            if (ia>=vertices.size()||ib>=vertices.size()||ic>=vertices.size())
                return Result<std::string>::failure({ErrorCode::invalid_argument,
                    "Scene mesh index references a missing vertex"});
            const auto& va=vertices[ia]; const auto& vb=vertices[ib]; const auto& vc=vertices[ic];
            const Vec3f pa{va.world.x,va.world.y,va.world.z};
            const Vec3f pb{vb.world.x,vb.world.y,vb.world.z};
            const Vec3f pc{vc.world.x,vc.world.y,vc.world.z};
            auto normal=cross(subtract(pb,pa),subtract(pc,pa));
            const float normal_len=length(normal);
            if (!std::isfinite(normal_len)||normal_len<=1.0e-8F) continue;
            normal={normal.x/normal_len,normal.y/normal_len,normal.z/normal_len};
            const float illumination=0.24F+0.76F*std::max(0.0F,dot(normal,light));
            const std::array<std::uint8_t,3> color{
                static_cast<std::uint8_t>(std::clamp(instance.diffuse_color.x*illumination*255.0F,0.0F,255.0F)),
                static_cast<std::uint8_t>(std::clamp(instance.diffuse_color.y*illumination*255.0F,0.0F,255.0F)),
                static_cast<std::uint8_t>(std::clamp(instance.diffuse_color.z*illumination*255.0F,0.0F,255.0F))};

            auto polygon=clip_polygon({va,vb,vc});
            if (polygon.size()<3U) continue;
            std::vector<RasterVertex> projected;
            projected.reserve(polygon.size());
            bool valid=true;
            for (const auto& v : polygon) {
                if (!std::isfinite(v.w)||v.w<=1.0e-8F) { valid=false; break; }
                const float nx=v.x/v.w, ny=v.y/v.w, nz=v.z/v.w;
                if (!std::isfinite(nx)||!std::isfinite(ny)||!std::isfinite(nz)) {
                    valid=false; break;
                }
                projected.push_back({
                    (nx*0.5F+0.5F)*static_cast<float>(width-1U),
                    (0.5F-ny*0.5F)*static_cast<float>(height-1U), nz});
            }
            if (!valid) continue;

            // A clipped triangle may become a convex quad or pentagon; fan triangulation is safe.
            for (std::size_t fan=1; fan+1<projected.size(); ++fan) {
                const auto& a=projected[0]; const auto& b=projected[fan]; const auto& c=projected[fan+1];
                const float area=(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
                if (!std::isfinite(area)||std::abs(area)<1.0e-8F) continue;
                const int x0=std::max(0,static_cast<int>(std::floor(std::min({a.x,b.x,c.x}))));
                const int y0=std::max(0,static_cast<int>(std::floor(std::min({a.y,b.y,c.y}))));
                const int x1=std::min(static_cast<int>(width)-1,static_cast<int>(std::ceil(std::max({a.x,b.x,c.x}))));
                const int y1=std::min(static_cast<int>(height)-1,static_cast<int>(std::ceil(std::max({a.y,b.y,c.y}))));
                for (int y=y0;y<=y1;++y) for (int x=x0;x<=x1;++x) {
                    const float px=static_cast<float>(x)+0.5F, py=static_cast<float>(y)+0.5F;
                    const float w0=((b.x-px)*(c.y-py)-(b.y-py)*(c.x-px))/area;
                    const float w1=((c.x-px)*(a.y-py)-(c.y-py)*(a.x-px))/area;
                    const float w2=1.0F-w0-w1;
                    if (w0<-1.0e-5F||w1<-1.0e-5F||w2<-1.0e-5F) continue;
                    const float z=w0*a.z+w1*b.z+w2*c.z;
                    if (!std::isfinite(z)||z < -1.00001F||z > 1.00001F) continue;
                    const auto index=static_cast<std::size_t>(y)*width+static_cast<std::size_t>(x);
                    if (z>=depth[index]) continue;
                    depth[index]=z;
                    pixels[index*3]=color[0]; pixels[index*3+1]=color[1]; pixels[index*3+2]=color[2];
                }
            }
        }
    }
    std::string image="P6\n"+std::to_string(width)+" "+std::to_string(height)+"\n255\n";
    image.append(reinterpret_cast<const char*>(pixels.data()),pixels.size());
    return Result<std::string>::success(std::move(image));
}

namespace {
bool path_is_within(const std::filesystem::path& base, const std::filesystem::path& candidate) {
    auto a = base.begin();
    auto b = candidate.begin();
    for (; a != base.end(); ++a, ++b) {
        if (b == candidate.end() || *a != *b) return false;
    }
    return true;
}

Result<void> append_scene_graph_instances(
    const SceneGraph& scene, const AssetRegistry& assets,
    const std::filesystem::path& root, const Matrix4& parent_transform,
    std::vector<MeshData>& meshes, std::vector<RenderMeshInstance>& instances,
    std::unordered_map<std::string, std::size_t>& mesh_cache) {
    constexpr std::uintmax_t max_asset_bytes = 16U * 1024U * 1024U;
    for (const auto& node : scene.nodes()) {
        if (node.mesh_asset_id.empty()) continue;
        const auto* asset = assets.find(node.mesh_asset_id);
        if (!asset) {
            return Result<void>::failure({ErrorCode::not_found,
                "Scene node '" + node.name + "' references missing mesh asset '" + node.mesh_asset_id + "'"});
        }
        if (asset->type != "mesh.obj") {
            return Result<void>::failure({ErrorCode::unsupported,
                "Asset '" + asset->id + "' is not a supported mesh.obj asset"});
        }

        auto cached = mesh_cache.find(asset->id);
        if (cached == mesh_cache.end()) {
            auto relative = platform::safe_relative_path(asset->path);
            if (!relative) return Result<void>::failure(relative.error());
            std::error_code ec;
            const auto source = std::filesystem::canonical(root / relative.value(), ec);
            if (ec) return Result<void>::failure({ErrorCode::io_error,
                "Cannot resolve mesh asset '" + asset->id + "': " + ec.message()});
            if (!path_is_within(root, source)) {
                return Result<void>::failure({ErrorCode::path_escape,
                    "Mesh asset resolves outside the configured asset root"});
            }
            const auto size = std::filesystem::file_size(source, ec);
            if (ec || size > max_asset_bytes) {
                return Result<void>::failure({ErrorCode::invalid_argument,
                    "Mesh asset is unreadable or exceeds the 16 MiB import limit"});
            }
            std::ifstream input(source, std::ios::binary);
            if (!input) return Result<void>::failure({ErrorCode::io_error,
                "Cannot open mesh asset: " + asset->id});
            std::string contents(static_cast<std::size_t>(size), '\0');
            if (size) input.read(contents.data(), static_cast<std::streamsize>(size));
            if (!input || input.peek() != std::char_traits<char>::eof()) {
                return Result<void>::failure({ErrorCode::io_error,
                    "Failed to read complete mesh asset: " + asset->id});
            }
            auto imported = import_obj_mesh(contents);
            if (!imported) return Result<void>::failure({imported.error().code,
                "Invalid mesh asset '" + asset->id + "': " + imported.error().message});
            const auto index = meshes.size();
            meshes.push_back(std::move(imported.value()));
            cached = mesh_cache.emplace(asset->id, index).first;
        }

        auto local_world = scene.world_matrix(node.id);
        if (!local_world) return Result<void>::failure(local_world.error());
        instances.push_back(RenderMeshInstance{
            &meshes[cached->second], multiply(parent_transform, local_world.value())});
    }
    return Result<void>::success();
}

Result<std::filesystem::path> canonical_asset_root(const std::filesystem::path& asset_root) {
    if (asset_root.empty()) {
        return Result<std::filesystem::path>::failure(
            {ErrorCode::invalid_argument, "Asset root must not be empty"});
    }
    std::error_code ec;
    const auto root = std::filesystem::canonical(asset_root, ec);
    if (ec || !std::filesystem::is_directory(root)) {
        return Result<std::filesystem::path>::failure(
            {ErrorCode::io_error, "Cannot resolve asset root"});
    }
    return Result<std::filesystem::path>::success(root);
}
} // namespace

Result<std::string> render_scene_graph_to_ppm(
    const SceneGraph& scene, const AssetRegistry& assets,
    const std::filesystem::path& asset_root,
    std::uint32_t width, std::uint32_t height) {
    auto root = canonical_asset_root(asset_root);
    if (!root) return Result<std::string>::failure(root.error());
    const auto nodes = scene.nodes();
    std::vector<MeshData> meshes;
    std::vector<RenderMeshInstance> instances;
    meshes.reserve(nodes.size());
    instances.reserve(nodes.size());
    std::unordered_map<std::string, std::size_t> mesh_cache;
    auto appended = append_scene_graph_instances(scene, assets, root.value(),
        Matrix4::identity(), meshes, instances, mesh_cache);
    if (!appended) return Result<std::string>::failure(appended.error());
    if (instances.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_state,
            "Scene has no nodes referencing renderable mesh assets"});
    }
    return render_scene_to_ppm(instances, width, height);
}

Result<std::string> render_world_content_runtime_to_ppm(
    const WorldContentRuntime& runtime, const AssetRegistry& assets,
    const std::filesystem::path& asset_root,
    std::uint32_t width, std::uint32_t height) {
    auto root = canonical_asset_root(asset_root);
    if (!root) return Result<std::string>::failure(root.error());

    struct ResidentScene {
        SceneGraph graph;
        Matrix4 world_transform;
        std::size_t node_count;
    };
    std::vector<ResidentScene> scenes;
    std::size_t total_nodes = 0;
    for (const auto& cell : runtime.manifest().cells) {
        const auto* status = runtime.world().find(cell.id);
        if (!status || status->state != CellState::loaded) continue;
        auto snapshot = runtime.scene_snapshot(cell.id);
        if (!snapshot) return Result<std::string>::failure(snapshot.error());
        const auto count = snapshot.value().size();
        if (count > std::numeric_limits<std::size_t>::max() - total_nodes) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Resident world contains too many scene nodes"});
        }
        total_nodes += count;
        auto cell_transform = Matrix4::identity();
        cell_transform.values[3] = static_cast<float>(cell.coord.x);
        cell_transform.values[7] = static_cast<float>(cell.coord.y);
        cell_transform.values[11] = static_cast<float>(cell.coord.z);
        scenes.push_back({std::move(snapshot.value()), cell_transform, count});
    }
    if (scenes.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_state,
            "Cannot render a world with no loaded scene cells"});
    }

    std::vector<MeshData> meshes;
    std::vector<RenderMeshInstance> instances;
    meshes.reserve(total_nodes);
    instances.reserve(total_nodes);
    std::unordered_map<std::string, std::size_t> mesh_cache;
    for (const auto& scene : scenes) {
        auto appended = append_scene_graph_instances(scene.graph, assets, root.value(),
            scene.world_transform, meshes, instances, mesh_cache);
        if (!appended) return Result<std::string>::failure(appended.error());
    }
    if (instances.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_state,
            "Loaded world cells contain no nodes referencing renderable mesh assets"});
    }
    return render_scene_to_ppm(instances, width, height);
}

Result<std::string> render_world_content_runtime_to_ppm(
    const WorldContentRuntime& runtime, const AssetRegistry& assets,
    const std::filesystem::path& asset_root, const Matrix4& view_projection,
    std::uint32_t width, std::uint32_t height) {
    auto root = canonical_asset_root(asset_root);
    if (!root) return Result<std::string>::failure(root.error());

    struct ResidentScene {
        SceneGraph graph;
        Matrix4 world_transform;
        std::size_t node_count;
    };
    std::vector<ResidentScene> scenes;
    std::size_t total_nodes = 0;
    for (const auto& cell : runtime.manifest().cells) {
        const auto* status = runtime.world().find(cell.id);
        if (!status || status->state != CellState::loaded) continue;
        auto snapshot = runtime.scene_snapshot(cell.id);
        if (!snapshot) return Result<std::string>::failure(snapshot.error());
        const auto count = snapshot.value().size();
        if (count > std::numeric_limits<std::size_t>::max() - total_nodes) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Resident world contains too many scene nodes"});
        }
        total_nodes += count;
        auto cell_transform = Matrix4::identity();
        cell_transform.values[3] = static_cast<float>(cell.coord.x);
        cell_transform.values[7] = static_cast<float>(cell.coord.y);
        cell_transform.values[11] = static_cast<float>(cell.coord.z);
        scenes.push_back({std::move(snapshot.value()), cell_transform, count});
    }
    if (scenes.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_state,
            "Cannot render a world with no loaded scene cells"});
    }

    std::vector<MeshData> meshes;
    std::vector<RenderMeshInstance> instances;
    meshes.reserve(total_nodes);
    instances.reserve(total_nodes);
    std::unordered_map<std::string, std::size_t> mesh_cache;
    for (const auto& scene : scenes) {
        auto appended = append_scene_graph_instances(scene.graph, assets, root.value(),
            scene.world_transform, meshes, instances, mesh_cache);
        if (!appended) return Result<std::string>::failure(appended.error());
    }
    if (instances.empty()) {
        return Result<std::string>::failure({ErrorCode::invalid_state,
            "Loaded world cells contain no nodes referencing renderable mesh assets"});
    }
    return render_scene_to_ppm(instances, view_projection, width, height);
}

Result<std::string> render_mesh_to_ppm(const MeshData& mesh, std::uint32_t width, std::uint32_t height) {
    constexpr std::uint32_t max_dimension = 4096;
    if (width == 0 || height == 0 || width > max_dimension || height > max_dimension) {
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "Render dimensions must be between 1 and 4096 pixels"});
    }
    if (mesh.positions.empty() || mesh.indices.empty() || mesh.indices.size() % 3 != 0) {
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "Renderer requires vertices and a triangle index buffer"});
    }
    for (const auto index : mesh.indices) {
        if (index >= mesh.positions.size()) {
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Mesh index references a missing vertex"});
        }
    }
    const auto pixel_count = static_cast<std::size_t>(width) * height;
    if (pixel_count > std::numeric_limits<std::size_t>::max() / 3) {
        return Result<std::string>::failure({ErrorCode::invalid_argument, "Image size overflows address space"});
    }

    float min_x = std::numeric_limits<float>::max();
    float min_y = min_x;
    float max_x = std::numeric_limits<float>::lowest();
    float max_y = max_x;
    for (const auto& p : mesh.positions) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
            return Result<std::string>::failure({ErrorCode::invalid_argument, "Mesh contains non-finite vertex coordinates"});
        }
        min_x = std::min(min_x,p.x); max_x = std::max(max_x,p.x);
        min_y = std::min(min_y,p.y); max_y = std::max(max_y,p.y);
    }
    const float span_x = max_x-min_x;
    const float span_y = max_y-min_y;
    const float span = std::max({span_x,span_y,0.0001F});
    if (!std::isfinite(span_x) || !std::isfinite(span_y) || !std::isfinite(span)) {
        return Result<std::string>::failure({ErrorCode::invalid_argument,
            "Mesh bounds exceed the renderer's numeric range"});
    }
    const float scale = 0.90F * static_cast<float>(std::min(width,height)) / span;
    const float center_x = min_x*0.5F+max_x*0.5F;
    const float center_y = min_y*0.5F+max_y*0.5F;
    std::vector<ScreenVertex> projected;
    projected.reserve(mesh.positions.size());
    for (const auto& p : mesh.positions) {
        projected.push_back({
            (static_cast<float>(width)*0.5F)+(p.x-center_x)*scale,
            (static_cast<float>(height)*0.5F)-(p.y-center_y)*scale,
            p.z
        });
    }

    // Dark neutral background and a depth buffer; this is a deterministic reference path.
    const std::array<std::uint8_t,3> background{18,22,30};
    std::vector<std::uint8_t> pixels(pixel_count*3);
    for (std::size_t i=0; i<pixel_count; ++i) {
        pixels[i*3]=background[0]; pixels[i*3+1]=background[1]; pixels[i*3+2]=background[2];
    }
    std::vector<float> depth(pixel_count, -std::numeric_limits<float>::infinity());
    const Vec3f light = [] {
        Vec3f v{-0.35F,0.65F,0.68F};
        const float len=length(v);
        return Vec3f{v.x/len,v.y/len,v.z/len};
    }();

    for (std::size_t tri=0; tri<mesh.indices.size(); tri+=3) {
        const auto& a=projected[mesh.indices[tri]];
        const auto& b=projected[mesh.indices[tri+1]];
        const auto& c=projected[mesh.indices[tri+2]];
        const Vec3f pa{mesh.positions[mesh.indices[tri]].x,mesh.positions[mesh.indices[tri]].y,mesh.positions[mesh.indices[tri]].z};
        const Vec3f pb{mesh.positions[mesh.indices[tri+1]].x,mesh.positions[mesh.indices[tri+1]].y,mesh.positions[mesh.indices[tri+1]].z};
        const Vec3f pc{mesh.positions[mesh.indices[tri+2]].x,mesh.positions[mesh.indices[tri+2]].y,mesh.positions[mesh.indices[tri+2]].z};
        auto normal=cross(subtract(pb,pa),subtract(pc,pa));
        const float normal_len=length(normal);
        if (normal_len <= 1.0e-8F) continue;
        normal={normal.x/normal_len,normal.y/normal_len,normal.z/normal_len};
        const float illumination=0.24F+0.76F*std::max(0.0F,dot(normal,light));
        const std::array<std::uint8_t,3> color{
            static_cast<std::uint8_t>(std::clamp(55.0F*illumination,0.0F,255.0F)),
            static_cast<std::uint8_t>(std::clamp(170.0F*illumination,0.0F,255.0F)),
            static_cast<std::uint8_t>(std::clamp(235.0F*illumination,0.0F,255.0F))};
        const float area=(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
        if (std::abs(area) < 1.0e-8F) continue;
        const int x0=std::max(0,static_cast<int>(std::floor(std::min({a.x,b.x,c.x}))));
        const int y0=std::max(0,static_cast<int>(std::floor(std::min({a.y,b.y,c.y}))));
        const int x1=std::min(static_cast<int>(width)-1,static_cast<int>(std::ceil(std::max({a.x,b.x,c.x}))));
        const int y1=std::min(static_cast<int>(height)-1,static_cast<int>(std::ceil(std::max({a.y,b.y,c.y}))));
        for (int y=y0; y<=y1; ++y) for (int x=x0; x<=x1; ++x) {
            const float px=static_cast<float>(x)+0.5F, py=static_cast<float>(y)+0.5F;
            const float w0=((b.x-px)*(c.y-py)-(b.y-py)*(c.x-px))/area;
            const float w1=((c.x-px)*(a.y-py)-(c.y-py)*(a.x-px))/area;
            const float w2=1.0F-w0-w1;
            if (w0 < -1.0e-5F || w1 < -1.0e-5F || w2 < -1.0e-5F) continue;
            const float z=w0*a.z+w1*b.z+w2*c.z;
            const auto offset=static_cast<std::size_t>(y)*width+static_cast<std::size_t>(x);
            if (z <= depth[offset]) continue;
            depth[offset]=z;
            pixels[offset*3]=color[0]; pixels[offset*3+1]=color[1]; pixels[offset*3+2]=color[2];
        }
    }
    std::string image;
    image.reserve(32+pixels.size());
    image += "P6\n" + std::to_string(width) + " " + std::to_string(height) + "\n255\n";
    image.append(reinterpret_cast<const char*>(pixels.data()),pixels.size());
    return Result<std::string>::success(std::move(image));
}
} // namespace ecos::studio
