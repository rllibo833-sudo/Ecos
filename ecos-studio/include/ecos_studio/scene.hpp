#pragma once

#include "ecos_studio/result.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ecos::studio {

struct Vec3 {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};
};

struct Transform {
    Vec3 position{};
    Vec3 rotation_euler_radians{};
    Vec3 scale{1.0F, 1.0F, 1.0F};
};

struct Matrix4 {
    std::array<float, 16> values{};
    [[nodiscard]] static Matrix4 identity() noexcept;
};

[[nodiscard]] Matrix4 compose(const Transform& transform) noexcept;
[[nodiscard]] Matrix4 multiply(const Matrix4& a, const Matrix4& b) noexcept;

using NodeId = std::uint64_t;

struct SceneNode {
    NodeId id{0};
    std::string name;
    std::string mesh_asset_id;
    Transform local_transform{};
    std::optional<NodeId> parent;
    std::vector<NodeId> children;
};

class SceneGraph {
public:
    [[nodiscard]] Result<NodeId> create_node(std::string name, std::optional<NodeId> parent = std::nullopt);
    [[nodiscard]] Result<void> set_parent(NodeId node, std::optional<NodeId> parent);
    [[nodiscard]] Result<void> set_local_transform(NodeId node, const Transform& transform);
    [[nodiscard]] Result<void> rename_node(NodeId node, std::string name);
    [[nodiscard]] Result<void> set_mesh_asset(NodeId node, std::string asset_id);
    [[nodiscard]] Result<void> restore_subtree(const std::vector<SceneNode>& snapshot);
    [[nodiscard]] Result<void> destroy_subtree(NodeId node);
    [[nodiscard]] const SceneNode* find(NodeId node) const noexcept;
    [[nodiscard]] SceneNode* find(NodeId node) noexcept;
    [[nodiscard]] Result<Matrix4> world_matrix(NodeId node) const;
    [[nodiscard]] std::vector<SceneNode> nodes() const;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] const std::vector<NodeId>& roots() const noexcept;

private:
    [[nodiscard]] bool is_ancestor(NodeId candidate, NodeId node) const noexcept;
    void detach_from_parent(SceneNode& node) noexcept;
    NodeId next_id_{1};
    std::unordered_map<NodeId, SceneNode> nodes_;
    std::vector<NodeId> roots_;
};

[[nodiscard]] Result<std::string> serialize_scene(const std::string& scene_id, const SceneGraph& graph);
[[nodiscard]] Result<SceneGraph> deserialize_scene(const std::string& text);

} // namespace ecos::studio
