#include "ecos_studio/render.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
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
    float min_z = min_x;
    float max_z = max_x;
    for (const auto& p : mesh.positions) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
            return Result<std::string>::failure({ErrorCode::invalid_argument, "Mesh contains non-finite vertex coordinates"});
        }
        min_x = std::min(min_x,p.x); max_x = std::max(max_x,p.x);
        min_y = std::min(min_y,p.y); max_y = std::max(max_y,p.y);
        min_z = std::min(min_z,p.z); max_z = std::max(max_z,p.z);
    }
    const float span_x = max_x-min_x;
    const float span_y = max_y-min_y;
    const float span = std::max({span_x,span_y,0.0001F});
    const float scale = 0.90F * static_cast<float>(std::min(width,height)) / span;
    const float center_x = (min_x+max_x)*0.5F;
    const float center_y = (min_y+max_y)*0.5F;
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
