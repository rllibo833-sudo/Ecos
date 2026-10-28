#include "ecos_studio/scene_editor.hpp"
#include <cmath>
#include <iostream>
namespace {
int failures = 0;
template <typename T> void check(const T& condition, const char* message) {
 if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void transform_history() {
 using namespace ecos::studio;
 SceneGraph scene; auto node = scene.create_node("Player");
 check(node, "creates node"); if (!node) return;
 SceneEditorHistory history(scene); check(!history.undo(), "empty undo returns error");
 Transform moved; moved.position = {3.0F, 4.0F, 5.0F};
 check(history.set_transform(node.value(), moved), "applies transform");
 check(history.can_undo() && history.undo_count() == 1, "records edit");
 check(std::fabs(scene.find(node.value())->local_transform.position.x - 3.0F) < 0.0001F, "new position applied");
 check(history.undo(), "undo succeeds");
 check(std::fabs(scene.find(node.value())->local_transform.position.x) < 0.0001F, "undo restores original transform");
 check(history.can_redo() && history.redo(), "redo succeeds");
 check(std::fabs(scene.find(node.value())->local_transform.position.y - 4.0F) < 0.0001F, "redo restores edited transform");
 Transform other; other.position = {-2.0F, 1.0F, 0.0F};
 check(history.set_transform(node.value(), other), "new edit succeeds");
 check(!history.can_redo(), "new edit invalidates redo branch");
 check(!history.set_transform(999999U, moved), "missing node is rejected");
}
void hierarchy_and_rename_history() {
 using namespace ecos::studio;
 SceneGraph scene; auto a = scene.create_node("RootA"); auto b = scene.create_node("RootB");
 auto child = scene.create_node("Child", a.value());
 check(a && b && child, "creates hierarchy"); if (!a || !b || !child) return;
 SceneEditorHistory history(scene);
 check(history.reparent(child.value(), b.value()), "reparents child");
 check(scene.find(child.value())->parent == b.value(), "parent changes");
 check(history.undo(), "undoes reparent");
 check(scene.find(child.value())->parent == a.value(), "undo restores parent");
 check(history.redo(), "redoes reparent");
 check(scene.find(child.value())->parent == b.value(), "redo restores parent");
 check(history.rename_node(child.value(), "Renamed"), "renames node");
 check(scene.find(child.value())->name == "Renamed", "rename is applied");
 check(history.undo() && scene.find(child.value())->name == "Child", "undo restores name");
 check(history.redo() && scene.find(child.value())->name == "Renamed", "redo restores renamed value");
 const auto before = history.undo_count();
 check(!history.reparent(b.value(), child.value()), "cyclic reparent is rejected");
 check(history.undo_count() == before, "failed edit is not recorded");
 history.clear(); check(!history.can_undo() && !history.can_redo(), "clear empties both stacks");
}
void create_delete_subtree_history() {
 using namespace ecos::studio;
 SceneGraph scene; auto parent = scene.create_node("WorldRoot");
 check(parent, "creates parent"); if (!parent) return;
 SceneEditorHistory history(scene);
 auto created = history.create_node("Player", parent.value());
 check(created, "editor creates node");
 if (!created) return;
 auto child = history.create_node("Weapon", created.value());
 check(child, "editor creates child"); if (!child) return;
 const auto created_id = created.value(), child_id = child.value();
 check(history.undo(), "undo removes latest created child");
 check(scene.find(child_id) == nullptr, "created child removed");
 check(history.undo(), "undo removes created parent");
 check(scene.find(created_id) == nullptr, "created node removed");
 check(history.redo(), "redo restores created parent");
 check(scene.find(created_id) != nullptr, "created node restored with stable id");
 check(history.redo(), "redo restores child");
 check(scene.find(child_id) && scene.find(child_id)->parent == created_id, "child restored with hierarchy");
 check(history.delete_subtree(created_id), "deletes subtree");
 check(scene.find(created_id) == nullptr && scene.find(child_id) == nullptr, "delete removes descendants");
 check(history.undo(), "undo restores deleted subtree");
 check(scene.find(created_id) && scene.find(child_id), "undo restores all original ids");
 check(scene.find(child_id)->parent == created_id, "undo restores internal parent link");
 check(scene.find(parent.value())->children.size() == 1, "restored subtree root is attached exactly once");
 check(history.redo(), "redo deletes subtree again");
 check(scene.find(created_id) == nullptr && scene.find(child_id) == nullptr, "redo removes subtree again");
 check(scene.find(parent.value()) != nullptr, "external parent remains intact");
 check(!history.delete_subtree(999999U), "missing delete target rejected");
}
}
int main() {
 transform_history(); hierarchy_and_rename_history(); create_delete_subtree_history();
 if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
 std::cout << "ECOS Studio scene editor history tests passed.\n"; return 0;
}
