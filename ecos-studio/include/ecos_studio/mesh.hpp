#pragma once

#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace ecos::studio {

struct Vec2 {
    float x{};
    float y{};
};

struct MeshBounds {
    Vec3 minimum{};
    Vec3 maximum{};
};

struct MeshData {
    // Vertex attributes are expanded so each index addresses one complete vertex.
    std::vector<Vec3> positions;
    std::vector<Vec2> texcoords;
    std::vector<Vec3> normals;
    std::vector<std::uint32_t> indices;
    MeshBounds bounds{};
};

[[nodiscard]] Result<MeshData> import_obj_mesh(std::string_view source);

} // namespace ecos::studio
