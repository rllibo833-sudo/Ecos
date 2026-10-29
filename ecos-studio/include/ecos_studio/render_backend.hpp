#pragma once

#include "ecos_studio/render.hpp"

#include <string_view>

namespace ecos::studio {

// Stable backend metadata lets the editor/runtime select a renderer without
// depending on a concrete graphics API.
struct RenderBackendInfo {
    std::string_view name;
    bool hardware_accelerated{false};
    bool supports_depth_buffer{false};
    bool supports_clip_space_clipping{false};
};

// Backend contract for producing one camera-projected frame. Mesh data remains
// owned by the asset/scene layer; implementations must not retain its pointers.
class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;

    [[nodiscard]] virtual RenderBackendInfo info() const noexcept = 0;
    [[nodiscard]] virtual Result<std::string> render_frame(
        const std::vector<RenderMeshInstance>& instances,
        const Matrix4& view_projection,
        std::uint32_t width, std::uint32_t height) const = 0;
};

// Deterministic software implementation used as the reference backend and as
// the fallback when a platform GPU backend is unavailable.
class CpuReferenceRenderBackend final : public IRenderBackend {
public:
    [[nodiscard]] RenderBackendInfo info() const noexcept override;
    [[nodiscard]] Result<std::string> render_frame(
        const std::vector<RenderMeshInstance>& instances,
        const Matrix4& view_projection,
        std::uint32_t width, std::uint32_t height) const override;
};

} // namespace ecos::studio
