#pragma once

#include "ecos_studio/mesh.hpp"
#include "ecos_studio/asset_registry.hpp"
#include "ecos_studio/result.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ecos::studio {

class WorldContentRuntime;

// Non-owning render input. Each instance contributes geometry transformed into a shared scene frame.
struct RenderMeshInstance {
    const MeshData* mesh{};
    Matrix4 world_transform{Matrix4::identity()};
};

// Deterministic CPU reference renderer. Returns a binary PPM (P6) image.
// The matrix overload applies a row-major world transform before rasterization.
[[nodiscard]] Result<std::string> render_mesh_to_ppm(
    const MeshData& mesh, std::uint32_t width, std::uint32_t height);
[[nodiscard]] Result<std::string> render_mesh_to_ppm(
    const MeshData& mesh, const Matrix4& world_transform,
    std::uint32_t width, std::uint32_t height);
[[nodiscard]] Result<std::string> render_scene_to_ppm(
    const std::vector<RenderMeshInstance>& instances,
    std::uint32_t width, std::uint32_t height);

[[nodiscard]] Result<std::string> render_scene_graph_to_ppm(
    const SceneGraph& scene, const AssetRegistry& assets,
    const std::filesystem::path& asset_root,
    std::uint32_t width, std::uint32_t height);
[[nodiscard]] Result<std::string> render_world_content_runtime_to_ppm(
    const WorldContentRuntime& runtime, const AssetRegistry& assets,
    const std::filesystem::path& asset_root,
    std::uint32_t width, std::uint32_t height);

} // namespace ecos::studio
