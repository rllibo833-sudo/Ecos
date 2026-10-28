#include "ecos_studio/render.hpp"
#include "ecos_studio/platform.hpp"

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


Result<std::string> render_scene_graph_to_ppm(
    const SceneGraph& scene, const AssetRegistry& assets,
    const std::filesystem::path& asset_root,
    std::uint32_t width, std::uint32_t height) {
    if (asset_root.empty())
        return Result<std::string>::failure({ErrorCode::invalid_argument, "Asset root must not be empty"});
    std::error_code ec;
    const auto root = std::filesystem::canonical(asset_root, ec);
    if (ec || !std::filesystem::is_directory(root))
        return Result<std::string>::failure({ErrorCode::io_error, "Cannot resolve asset root"});
    auto within_root = [](const std::filesystem::path& base, const std::filesystem::path& candidate) {
        auto a = base.begin(); auto b = candidate.begin();
        for (; a != base.end(); ++a, ++b) if (b == candidate.end() || *a != *b) return false;
        return true;
    };
    const auto nodes = scene.nodes();
    std::vector<MeshData> meshes;
    std::vector<RenderMeshInstance> instances;
    meshes.reserve(nodes.size());
    instances.reserve(nodes.size());
    constexpr std::uintmax_t max_asset_bytes = 16U * 1024U * 1024U;
    for (const auto& node : nodes) {
        if (node.mesh_asset_id.empty()) continue;
        const auto* asset = assets.find(node.mesh_asset_id);
        if (!asset)
            return Result<std::string>::failure({ErrorCode::not_found,
                "Scene node '" + node.name + "' references missing mesh asset '" + node.mesh_asset_id + "'"});
        if (asset->type != "mesh.obj")
            return Result<std::string>::failure({ErrorCode::unsupported,
                "Asset '" + asset->id + "' is not a supported mesh.obj asset"});
        auto relative = platform::safe_relative_path(asset->path);
        if (!relative) return Result<std::string>::failure(relative.error());
        const auto source = std::filesystem::canonical(root / relative.value(), ec);
        if (ec) return Result<std::string>::failure({ErrorCode::io_error,
            "Cannot resolve mesh asset '" + asset->id + "': " + ec.message()});
        if (!within_root(root, source))
            return Result<std::string>::failure({ErrorCode::path_escape,
                "Mesh asset resolves outside the configured asset root"});
        const auto size = std::filesystem::file_size(source, ec);
        if (ec || size > max_asset_bytes)
            return Result<std::string>::failure({ErrorCode::invalid_argument,
                "Mesh asset is unreadable or exceeds the 16 MiB import limit"});
        std::ifstream input(source, std::ios::binary);
        if (!input) return Result<std::string>::failure({ErrorCode::io_error,
            "Cannot open mesh asset: " + asset->id});
        std::string contents(static_cast<std::size_t>(size), '\0');
        if (size) input.read(contents.data(), static_cast<std::streamsize>(size));
        if (!input || input.peek() != std::char_traits<char>::eof())
            return Result<std::string>::failure({ErrorCode::io_error,
                "Failed to read complete mesh asset: " + asset->id});
        auto mesh = import_obj_mesh(contents);
        if (!mesh) return Result<std::string>::failure({mesh.error().code,
            "Invalid mesh asset '" + asset->id + "': " + mesh.error().message});
        auto world = scene.world_matrix(node.id);
        if (!world) return Result<std::string>::failure(world.error());
        meshes.push_back(std::move(mesh.value()));
        instances.push_back(RenderMeshInstance{&meshes.back(), world.value()});
    }
    if (instances.empty())
        return Result<std::string>::failure({ErrorCode::invalid_state,
            "Scene has no nodes referencing renderable mesh assets"});
    return render_scene_to_ppm(instances, width, height);
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
