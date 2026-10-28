#include "ecos_studio/scene_editor.hpp"
#include <utility>
namespace ecos::studio {
std::vector<SceneNode> SceneEditorHistory::capture_subtree(NodeId root) const {
 std::vector<SceneNode> snapshot;
 std::vector<NodeId> pending{root};
 while (!pending.empty()) {
  const NodeId id = pending.back(); pending.pop_back();
  const auto* node = scene_.find(id);
  if (!node) continue;
  snapshot.push_back(*node);
  pending.insert(pending.end(), node->children.begin(), node->children.end());
 }
 return snapshot;
}
Result<NodeId> SceneEditorHistory::create_node(std::string name, std::optional<NodeId> parent) {
 auto created = scene_.create_node(std::move(name), parent);
 if (!created) return created;
 Command command; command.kind = Kind::create; command.node = created.value();
 command.subtree = capture_subtree(command.node);
 undo_.push_back(std::move(command)); redo_.clear();
 return created;
}
Result<void> SceneEditorHistory::delete_subtree(NodeId id) {
 if (!scene_.find(id)) return Result<void>::failure({ErrorCode::not_found, "Cannot delete: scene node does not exist"});
 Command command; command.kind = Kind::erase; command.node = id;
 command.subtree = capture_subtree(id);
 auto result = apply(command, true); if (!result) return result;
 undo_.push_back(std::move(command)); redo_.clear(); return Result<void>::success();
}
Result<void> SceneEditorHistory::rename_node(NodeId id, std::string name) {
 const auto* node = scene_.find(id);
 if (!node) return Result<void>::failure({ErrorCode::not_found, "Cannot rename: scene node does not exist"});
 if (node->name == name) return Result<void>::success();
 Command command; command.kind = Kind::rename; command.node = id;
 command.before_name = node->name; command.after_name = std::move(name);
 auto result = apply(command, true); if (!result) return result;
 undo_.push_back(std::move(command)); redo_.clear(); return Result<void>::success();
}
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
 switch (command.kind) {
 case Kind::create:
  return forward ? scene_.restore_subtree(command.subtree) : scene_.destroy_subtree(command.node);
 case Kind::erase:
  return forward ? scene_.destroy_subtree(command.node) : scene_.restore_subtree(command.subtree);
 case Kind::rename:
  return scene_.rename_node(command.node, forward ? command.after_name : command.before_name);
 case Kind::transform:
  return scene_.set_local_transform(command.node, forward ? command.after_transform : command.before_transform);
 case Kind::parent:
  return scene_.set_parent(command.node, forward ? command.after_parent : command.before_parent);
 }
 return Result<void>::failure({ErrorCode::internal_error, "Unknown scene editor command"});
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
