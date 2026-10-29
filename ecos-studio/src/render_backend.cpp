#include "ecos_studio/render_backend.hpp"

namespace ecos::studio {

RenderBackendInfo CpuReferenceRenderBackend::info() const noexcept {
    return {
        "cpu-reference",
        false, // Software rasterization; no graphics device required.
        true,  // The reference rasterizer uses a depth buffer.
        true   // Homogeneous clipping against all six clip planes.
    };
}

Result<std::string> CpuReferenceRenderBackend::render_frame(
    const std::vector<RenderMeshInstance>& instances,
    const Matrix4& view_projection,
    std::uint32_t width, std::uint32_t height) const {
    return render_scene_to_ppm(instances, view_projection, width, height);
}

} // namespace ecos::studio
