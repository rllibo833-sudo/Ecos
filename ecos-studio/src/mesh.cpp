#include "ecos_studio/mesh.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ecos::studio {
namespace {
bool parse_float(const std::string& text, float& value) {
    if (text.empty()) return false;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() && std::isfinite(value);
}
bool parse_index(const std::string& token, std::int64_t& index) {
    const auto slash = token.find('/');
    const auto value = token.substr(0, slash);
    if (value.empty()) return false;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), index);
    return parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size() && index != 0;
}
}
Result<MeshData> import_obj_mesh(std::string_view source) {
    MeshData mesh;
    std::istringstream input{std::string(source)};
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        std::istringstream row(line);
        std::string kind;
        row >> kind;
        if (kind.empty() || kind[0] == '#') continue;
        if (kind == "v") {
            std::string sx, sy, sz;
            float x{}, y{}, z{};
            if (!(row >> sx >> sy >> sz) || !parse_float(sx, x) ||
                !parse_float(sy, y) || !parse_float(sz, z)) {
                return Result<MeshData>::failure({ErrorCode::invalid_argument,
                    "Invalid OBJ vertex at line " + std::to_string(line_number)});
            }
            mesh.positions.push_back({x, y, z});
        } else if (kind == "f") {
            std::vector<std::uint32_t> face;
            std::string token;
            while (row >> token) {
                if (!token.empty() && token[0] == '#') break;
                std::int64_t obj_index{};
                if (!parse_index(token, obj_index)) {
                    return Result<MeshData>::failure({ErrorCode::invalid_argument,
                        "Invalid OBJ face index at line " + std::to_string(line_number)});
                }
                const auto count = static_cast<std::int64_t>(mesh.positions.size());
                const std::int64_t resolved = obj_index > 0 ? obj_index - 1 : count + obj_index;
                if (resolved < 0 || resolved >= count) {
                    return Result<MeshData>::failure({ErrorCode::invalid_argument,
                        "OBJ face references a missing vertex at line " + std::to_string(line_number)});
                }
                face.push_back(static_cast<std::uint32_t>(resolved));
            }
            if (face.size() < 3) {
                return Result<MeshData>::failure({ErrorCode::invalid_argument,
                    "OBJ face must contain at least three vertices at line " + std::to_string(line_number)});
            }
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                mesh.indices.push_back(face[0]);
                mesh.indices.push_back(face[i]);
                mesh.indices.push_back(face[i + 1]);
            }
        }
    }
    if (mesh.positions.empty()) return Result<MeshData>::failure({ErrorCode::invalid_argument, "OBJ mesh contains no vertices"});
    if (mesh.indices.empty()) return Result<MeshData>::failure({ErrorCode::invalid_argument, "OBJ mesh contains no faces"});

    const float high = std::numeric_limits<float>::max();
    const float low = std::numeric_limits<float>::lowest();
    mesh.bounds.minimum = {high, high, high};
    mesh.bounds.maximum = {low, low, low};
    for (const auto& position : mesh.positions) {
        mesh.bounds.minimum.x = std::min(mesh.bounds.minimum.x, position.x);
        mesh.bounds.minimum.y = std::min(mesh.bounds.minimum.y, position.y);
        mesh.bounds.minimum.z = std::min(mesh.bounds.minimum.z, position.z);
        mesh.bounds.maximum.x = std::max(mesh.bounds.maximum.x, position.x);
        mesh.bounds.maximum.y = std::max(mesh.bounds.maximum.y, position.y);
        mesh.bounds.maximum.z = std::max(mesh.bounds.maximum.z, position.z);
    }
    return Result<MeshData>::success(std::move(mesh));
}
} // namespace ecos::studio
