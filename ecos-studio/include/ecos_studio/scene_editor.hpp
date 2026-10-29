#pragma once
#include "ecos_studio/scene.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
namespace ecos::studio {
class SceneEditorSelection {
public:
 explicit SceneEditorSelection(const SceneGraph& scene) noexcept : scene_(scene) {}
 [[nodiscard]] Result<void> select(NodeId node);
 [[nodiscard]] Result<void> add(NodeId node);
 [[nodiscard]] Result<void> toggle(NodeId node);
 void clear() noexcept;
 void prune_deleted();
 [[nodiscard]] bool contains(NodeId node) const noexcept;
 [[nodiscard]] const std::vector<NodeId>& selected_nodes() const noexcept { return selected_; }
 [[nodiscard]] std::optional<NodeId> active_node() const noexcept { return active_; }
private:
 const SceneGraph& scene_;
 std::vector<NodeId> selected_;
 std::optional<NodeId> active_;
};
class SceneEditorHistory {
public:
 explicit SceneEditorHistory(SceneGraph& scene) noexcept : scene_(scene) {}
 [[nodiscard]] Result<NodeId> create_node(std::string name, std::optional<NodeId> parent = std::nullopt);
 [[nodiscard]] Result<void> delete_subtree(NodeId node);
 [[nodiscard]] Result<void> rename_node(NodeId node, std::string name);
 [[nodiscard]] Result<void> set_transform(NodeId node, const Transform& transform);
 [[nodiscard]] Result<void> reparent(NodeId node, std::optional<NodeId> parent);
 [[nodiscard]] Result<void> set_mesh_asset(NodeId node, std::string asset_id);
 [[nodiscard]] Result<void> set_material_asset(NodeId node, std::string asset_id);
 [[nodiscard]] Result<void> undo();
 [[nodiscard]] Result<void> redo();
 [[nodiscard]] bool can_undo() const noexcept { return !undo_.empty(); }
 [[nodiscard]] bool can_redo() const noexcept { return !redo_.empty(); }
 [[nodiscard]] std::size_t undo_count() const noexcept { return undo_.size(); }
 [[nodiscard]] std::size_t redo_count() const noexcept { return redo_.size(); }
 void clear() noexcept;
private:
 enum class Kind { create, erase, rename, transform, parent, mesh_asset, material_asset };
 struct Command {
  Kind kind{Kind::transform}; NodeId node{0};
  Transform before_transform{}; Transform after_transform{};
  std::optional<NodeId> before_parent; std::optional<NodeId> after_parent;
  std::string before_name; std::string after_name;
  std::string before_asset_id; std::string after_asset_id;
  std::vector<SceneNode> subtree;
 };
 [[nodiscard]] Result<void> apply(const Command& command, bool forward);
 [[nodiscard]] std::vector<SceneNode> capture_subtree(NodeId root) const;
 SceneGraph& scene_; std::vector<Command> undo_; std::vector<Command> redo_;
};
} // namespace ecos::studio
