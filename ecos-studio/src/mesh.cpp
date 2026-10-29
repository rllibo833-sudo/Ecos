#include "ecos_studio/mesh.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace ecos::studio {
namespace {
bool parse_float(const std::string& text, float& value) {
    if (text.empty()) return false;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() && std::isfinite(value);
}
bool parse_integer(const std::string& text, std::int64_t& value) {
    if (text.empty()) return false;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() && value != 0;
}
bool resolve_index(std::int64_t obj_index, std::size_t count, std::int64_t& resolved) {
    if (count > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())) return false;
    const auto signed_count = static_cast<std::int64_t>(count);
    resolved = obj_index > 0 ? obj_index - 1 : signed_count + obj_index;
    return resolved >= 0 && resolved < signed_count;
}
struct ObjCorner {
    std::int64_t position{-1};
    std::int64_t texcoord{-1};
    std::int64_t normal{-1};
};
bool parse_corner(const std::string& token, std::size_t positions, std::size_t texcoords,
                  std::size_t normals, ObjCorner& corner) {
    const auto first = token.find('/');
    const auto second = first == std::string::npos ? std::string::npos : token.find('/', first + 1);
    if (second != std::string::npos && token.find('/', second + 1) != std::string::npos) return false;
    const auto p = token.substr(0, first);
    std::int64_t raw{};
    if (!parse_integer(p, raw) || !resolve_index(raw, positions, corner.position)) return false;
    if (first == std::string::npos) return true;
    const auto t = token.substr(first + 1, second == std::string::npos ? std::string::npos : second-first-1);
    // OBJ face corners allow v, v/vt, v//vn, and v/vt/vn. A slash
    // that introduces a missing final field is malformed.
    if (second == std::string::npos && t.empty()) return false;
    if (!t.empty()) {
        if (!parse_integer(t, raw) || !resolve_index(raw, texcoords, corner.texcoord)) return false;
    }
    if (second != std::string::npos) {
        const auto n = token.substr(second + 1);
        if (n.empty()) return false;
        if (!parse_integer(n, raw) || !resolve_index(raw, normals, corner.normal)) return false;
    }
    return true;
}
Vec3 subtract(Vec3 a, Vec3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
float length_squared(Vec3 v) { return v.x*v.x+v.y*v.y+v.z*v.z; }
}

Result<MeshData> import_obj_mesh(std::string_view source) {
    MeshData mesh;
    std::vector<Vec3> source_positions;
    std::vector<Vec2> source_texcoords;
    std::vector<Vec3> source_normals;
    using CornerKey = std::tuple<std::int64_t, std::int64_t, std::int64_t>;
    std::map<CornerKey, std::uint32_t> vertex_lookup;
    std::istringstream input{std::string(source)};
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        std::istringstream row(line);
        std::string kind;
        row >> kind;
        if (kind.empty() || kind[0] == '#') continue;
        if (kind == "v" || kind == "vn") {
            std::string sx, sy, sz;
            float x{}, y{}, z{};
            if (!(row >> sx >> sy >> sz) || !parse_float(sx, x) ||
                !parse_float(sy, y) || !parse_float(sz, z)) {
                return Result<MeshData>::failure({ErrorCode::invalid_argument,
                    "Invalid OBJ " + kind + " at line " + std::to_string(line_number)});
            }
            if (kind == "v") source_positions.push_back({x,y,z});
            else source_normals.push_back({x,y,z});
        } else if (kind == "vt") {
            std::string sx, sy;
            float x{}, y{};
            if (!(row >> sx >> sy) || !parse_float(sx,x) || !parse_float(sy,y)) {
                return Result<MeshData>::failure({ErrorCode::invalid_argument,
                    "Invalid OBJ texture coordinate at line " + std::to_string(line_number)});
            }
            source_texcoords.push_back({x,y});
        } else if (kind == "f") {
            std::vector<std::uint32_t> face;
            std::string token;
            while (row >> token) {
                if (!token.empty() && token[0] == '#') break;
                ObjCorner corner;
                if (!parse_corner(token, source_positions.size(), source_texcoords.size(),
                                  source_normals.size(), corner)) {
                    return Result<MeshData>::failure({ErrorCode::invalid_argument,
                        "Invalid OBJ face vertex reference at line " + std::to_string(line_number)});
                }
                const CornerKey key{corner.position,corner.texcoord,corner.normal};
                auto found = vertex_lookup.find(key);
                if (found == vertex_lookup.end()) {
                    if (mesh.positions.size() >= std::numeric_limits<std::uint32_t>::max()) {
                        return Result<MeshData>::failure({ErrorCode::invalid_argument,"OBJ mesh exceeds 32-bit vertex index capacity"});
                    }
                    const auto index=static_cast<std::uint32_t>(mesh.positions.size());
                    mesh.positions.push_back(source_positions[static_cast<std::size_t>(corner.position)]);
                    mesh.texcoords.push_back(corner.texcoord >= 0 ? source_texcoords[static_cast<std::size_t>(corner.texcoord)] : Vec2{});
                    mesh.normals.push_back(corner.normal >= 0 ? source_normals[static_cast<std::size_t>(corner.normal)] : Vec3{});
                    vertex_lookup.emplace(key,index);
                    face.push_back(index);
                } else face.push_back(found->second);
            }
            if (face.size() < 3) {
                return Result<MeshData>::failure({ErrorCode::invalid_argument,
                    "OBJ face must contain at least three vertices at line " + std::to_string(line_number)});
            }
            for (std::size_t i=1; i+1<face.size(); ++i) {
                mesh.indices.push_back(face[0]); mesh.indices.push_back(face[i]); mesh.indices.push_back(face[i+1]);
            }
        }
    }
    if (source_positions.empty()) return Result<MeshData>::failure({ErrorCode::invalid_argument,"OBJ mesh contains no vertices"});
    if (mesh.positions.empty() || mesh.indices.empty()) return Result<MeshData>::failure({ErrorCode::invalid_argument,"OBJ mesh contains no faces"});

    // Generate smooth, area-weighted normals only where the source omitted them.
    // Explicit OBJ normals remain untouched, including intentional hard edges.
    std::vector<Vec3> generated(mesh.positions.size());
    std::vector<bool> needs_normal(mesh.positions.size(), false);
    for (std::size_t i=0; i<mesh.normals.size(); ++i)
        needs_normal[i] = length_squared(mesh.normals[i]) <= 1.0e-20F;
    for (std::size_t i=0; i<mesh.indices.size(); i+=3) {
        const auto ia=mesh.indices[i], ib=mesh.indices[i+1], ic=mesh.indices[i+2];
        const auto face_normal=cross(
            subtract(mesh.positions[ib],mesh.positions[ia]),
            subtract(mesh.positions[ic],mesh.positions[ia]));
        if (!std::isfinite(face_normal.x) || !std::isfinite(face_normal.y) || !std::isfinite(face_normal.z))
            return Result<MeshData>::failure({ErrorCode::invalid_argument,"OBJ triangle exceeds normal-generation numeric range"});
        for (const auto index : {ia,ib,ic}) if (needs_normal[index]) {
            generated[index].x += face_normal.x;
            generated[index].y += face_normal.y;
            generated[index].z += face_normal.z;
        }
    }
    for (std::size_t i=0; i<mesh.normals.size(); ++i) if (needs_normal[i]) {
        const float length2=length_squared(generated[i]);
        if (length2 > 1.0e-20F && std::isfinite(length2)) {
            const float inverse=1.0F/std::sqrt(length2);
            mesh.normals[i]={generated[i].x*inverse,generated[i].y*inverse,generated[i].z*inverse};
        }
    }

    const float high=std::numeric_limits<float>::max();
    const float low=std::numeric_limits<float>::lowest();
    mesh.bounds.minimum={high,high,high};
    mesh.bounds.maximum={low,low,low};
    for (const auto& p:mesh.positions) {
        mesh.bounds.minimum.x=std::min(mesh.bounds.minimum.x,p.x);
        mesh.bounds.minimum.y=std::min(mesh.bounds.minimum.y,p.y);
        mesh.bounds.minimum.z=std::min(mesh.bounds.minimum.z,p.z);
        mesh.bounds.maximum.x=std::max(mesh.bounds.maximum.x,p.x);
        mesh.bounds.maximum.y=std::max(mesh.bounds.maximum.y,p.y);
        mesh.bounds.maximum.z=std::max(mesh.bounds.maximum.z,p.z);
    }
    return Result<MeshData>::success(std::move(mesh));
}
} // namespace ecos::studio
