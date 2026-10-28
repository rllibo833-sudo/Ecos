#pragma once

#include "ecos_studio/mesh.hpp"
#include "ecos_studio/result.hpp"

#include <cstdint>
#include <string>

namespace ecos::studio {

// Deterministic CPU reference renderer. Returns a binary PPM (P6) image.
[[nodiscard]] Result<std::string> render_mesh_to_ppm(
    const MeshData& mesh, std::uint32_t width, std::uint32_t height);

} // namespace ecos::studio
