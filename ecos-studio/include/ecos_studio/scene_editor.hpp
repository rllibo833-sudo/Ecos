#pragma once
#include "ecos_studio/scene.hpp"
#include <cstddef>
#include <optional>
#include <vector>
namespace ecos::studio {
class SceneEditorHistory {
public:
 explicit SceneEditorHistory(SceneGraph& scene) noexcept : scene_(scene) {}
 [[nodiscard]] Result<void> set_transform(NodeId node, const Transform& transform);
 [[nodiscard]] Result<void> reparent(NodeId node, std::optional<NodeId> parent);
 [[nodiscard]] Result<void> undo();
 [[nodiscard]] Result<void> redo();
 [[nodiscard]] bool can_undo() const noexcept { return !undo_.empty(); }
 [[nodiscard]] bool can_redo() const noexcept { return !redo_.empty(); }
 [[nodiscard]] std::size_t undo_count() const noexcept { return undo_.size(); }
 [[nodiscard]] std::size_t redo_count() const noexcept { return redo_.size(); }
 void clear() noexcept;
private:
 enum class Kind { transform, parent };
 struct Command {
  Kind kind{Kind::transform}; NodeId node{0};
  Transform before_transform{}; Transform after_transform{};
  std::optional<NodeId> before_parent; std::optional<NodeId> after_parent;
 };
 [[nodiscard]] Result<void> apply(const Command& command, bool forward);
 SceneGraph& scene_; std::vector<Command> undo_; std::vector<Command> redo_;
};
} // namespace ecos::studio
