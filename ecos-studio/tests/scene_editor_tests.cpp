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
void parent_history() {
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
 const auto before = history.undo_count();
 check(!history.reparent(b.value(), child.value()), "cyclic reparent is rejected");
 check(history.undo_count() == before, "failed edit is not recorded");
 history.clear(); check(!history.can_undo() && !history.can_redo(), "clear empties both stacks");
}
}
int main() {
 transform_history(); parent_history();
 if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
 std::cout << "ECOS Studio scene editor history tests passed.\n"; return 0;
}
