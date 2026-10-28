#pragma once

#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace ecos::studio {

struct MeshBounds {
    Vec3 minimum{};
    Vec3 maximum{};
};

struct MeshData {
    std::vector<Vec3> positions;
    std::vector<std::uint32_t> indices;
    MeshBounds bounds{};
};

[[nodiscard]] Result<MeshData> import_obj_mesh(std::string_view source);

} // namespace ecos::studio
