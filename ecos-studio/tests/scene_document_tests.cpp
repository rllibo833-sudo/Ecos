#include "ecos_studio/scene_document.hpp"

#include <iostream>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_document_edit_lifecycle() {
    using namespace ecos::studio;
    SceneDocument document("main-world");
    check(!document.is_dirty(), "new document starts clean");
    auto root = document.create_node("World");
    check(root && document.is_dirty(), "create marks document dirty");
    if (!root) return;
    check(document.revision() == 1, "successful edit advances revision");

    auto child = document.create_node("Player", root.value());
    check(child, "creates child through document");
    if (!child) return;
    check(document.selection().select(child.value()), "selection belongs to document");
    Transform transform;
    transform.position = {2.0F, 3.0F, 4.0F};
    check(document.set_transform(child.value(), transform), "edits transform through document");
    check(document.undo(), "document undo succeeds");
    check(document.scene().find(child.value())->local_transform.position.x == 0.0F,
          "undo updates document scene");
    check(document.redo(), "document redo succeeds");
    check(document.scene().find(child.value())->local_transform.position.x == 2.0F,
          "redo updates document scene");

    auto saved = document.save();
    check(saved && !document.is_dirty(), "successful serialization marks document clean");
    check(document.saved_revision() == document.revision(), "saved revision matches current revision");
    if (!saved) return;

    check(document.rename_node(child.value(), "Hero"), "post-save edit succeeds");
    check(document.is_dirty(), "post-save edit marks document dirty");
    check(document.load(saved.value()), "loads previously serialized scene");
    check(!document.is_dirty() && document.revision() == 0, "load resets revision and dirty state");
    check(document.scene_id() == "main-world", "load restores scene id");
    check(document.scene().size() == 2, "load replaces scene graph");
    check(document.scene().find(child.value())->name == "Player", "load restores saved node data");
    check(!document.history().can_undo(), "load clears stale undo history");
    check(document.selection().selected_nodes().empty(), "load clears stale selection");
}

void test_document_failure_is_atomic() {
    using namespace ecos::studio;
    SceneDocument document("before");
    auto root = document.create_node("World");
    check(root, "creates initial scene");
    if (!root) return;
    auto saved = document.save();
    check(saved, "saves initial scene");
    const auto old_revision = document.revision();
    const auto old_name = document.scene().find(root.value())->name;

    check(!document.load("format_version=1\nscene_id=bad\nnode=1|99|Broken|0,0,0|0,0,0|1,1,1\n"),
          "rejects invalid replacement scene");
    check(document.scene().find(root.value()) && document.scene().find(root.value())->name == old_name,
          "failed load leaves current graph untouched");
    check(document.revision() == old_revision && !document.is_dirty(),
          "failed load preserves revision and clean state");

    check(!document.rename_node(999999U, "Missing"), "failed edit is rejected");
    check(document.revision() == old_revision && !document.is_dirty(),
          "failed edit does not dirty document");
    SceneDocument invalid_id("");
    check(!invalid_id.save(), "invalid scene id fails save");
    check(invalid_id.is_dirty() == false, "failed save does not alter dirty state");
}
}

int main() {
    test_document_edit_lifecycle();
    test_document_failure_is_atomic();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "ECOS Studio scene document tests passed.\n";
    return 0;
}
