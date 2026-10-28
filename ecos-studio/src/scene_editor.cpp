#include "ecos_studio/scene_editor.hpp"
#include <utility>
namespace ecos::studio {
Result<void> SceneEditorHistory::set_transform(NodeId id, const Transform& transform) {
 const auto* node = scene_.find(id);
 if (!node) return Result<void>::failure({ErrorCode::not_found, "Cannot edit transform: scene node does not exist"});
 Command command; command.kind = Kind::transform; command.node = id;
 command.before_transform = node->local_transform; command.after_transform = transform;
 auto result = apply(command, true); if (!result) return result;
 undo_.push_back(std::move(command)); redo_.clear(); return Result<void>::success();
}
Result<void> SceneEditorHistory::reparent(NodeId id, std::optional<NodeId> parent) {
 const auto* node = scene_.find(id);
 if (!node) return Result<void>::failure({ErrorCode::not_found, "Cannot reparent: scene node does not exist"});
 if (node->parent == parent) return Result<void>::success();
 Command command; command.kind = Kind::parent; command.node = id;
 command.before_parent = node->parent; command.after_parent = parent;
 auto result = apply(command, true); if (!result) return result;
 undo_.push_back(std::move(command)); redo_.clear(); return Result<void>::success();
}
Result<void> SceneEditorHistory::apply(const Command& command, bool forward) {
 if (command.kind == Kind::transform)
  return scene_.set_local_transform(command.node, forward ? command.after_transform : command.before_transform);
 return scene_.set_parent(command.node, forward ? command.after_parent : command.before_parent);
}
Result<void> SceneEditorHistory::undo() {
 if (undo_.empty()) return Result<void>::failure({ErrorCode::invalid_state, "No scene edit is available to undo"});
 const Command& command = undo_.back(); auto result = apply(command, false); if (!result) return result;
 redo_.push_back(command); undo_.pop_back(); return Result<void>::success();
}
Result<void> SceneEditorHistory::redo() {
 if (redo_.empty()) return Result<void>::failure({ErrorCode::invalid_state, "No scene edit is available to redo"});
 const Command& command = redo_.back(); auto result = apply(command, true); if (!result) return result;
 undo_.push_back(command); redo_.pop_back(); return Result<void>::success();
}
void SceneEditorHistory::clear() noexcept { undo_.clear(); redo_.clear(); }
} // namespace ecos::studio
