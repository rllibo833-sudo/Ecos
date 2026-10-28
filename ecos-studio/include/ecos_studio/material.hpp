#pragma once

#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace ecos::studio {

struct MaterialData {
    std::string name;
    Vec3 diffuse_color{0.8F,0.8F,0.8F};
    float opacity{1.0F};
    float shininess{0.0F};
    int illumination_model{2};
};

// Parses the commonly used Wavefront MTL properties Kd, d, Tr, Ns, and illum.
[[nodiscard]] Result<std::vector<MaterialData>> import_mtl_materials(std::string_view source);

} // namespace ecos::studio
