#include "ecos_studio/asset_registry.hpp"
#include "ecos_studio/scene.hpp"

#include <cmath>
#include <iostream>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_scene_graph() {
    using namespace ecos::studio;
    SceneGraph graph;
    auto root = graph.create_node("World");
    auto child = graph.create_node("Player", root.value());
    auto grandchild = graph.create_node("Camera", child.value());
    check(root && child && grandchild, "creates hierarchical nodes");
    check(graph.size() == 3, "tracks node count");
    Transform root_transform;
    root_transform.position = {10.0F, 0.0F, 0.0F};
    check(graph.set_local_transform(root.value(), root_transform), "sets root transform");
    Transform child_transform;
    child_transform.position = {0.0F, 2.0F, 0.0F};
    check(graph.set_local_transform(child.value(), child_transform), "sets child transform");
    auto world = graph.world_matrix(child.value());
    check(world, "computes child world matrix");
    check(std::fabs(world.value().values[3] - 10.0F) < 0.0001F, "inherits parent x translation");
    check(std::fabs(world.value().values[7] - 2.0F) < 0.0001F, "keeps child y translation");
    check(!graph.set_parent(root.value(), grandchild.value()), "rejects cyclic parenting");
    check(!graph.create_node("bad", 999999U), "rejects missing parent");
    check(!graph.create_node(""), "rejects empty node name");
    check(graph.set_parent(child.value(), std::nullopt), "can detach a node to root");
    check(graph.roots().size() == 2, "root list updates after detach");
    check(graph.destroy_subtree(child.value()), "destroys node subtree");
    check(graph.find(child.value()) == nullptr && graph.find(grandchild.value()) == nullptr,
          "subtree destruction removes descendants");
    check(graph.size() == 1, "subtree removal updates count");
}
void test_asset_registry() {
    using namespace ecos::studio;
    AssetRegistry registry;
    check(registry.register_asset({"mesh.tree", "mesh", "assets/tree.mesh", {}}), "registers mesh asset");
    check(registry.register_asset({"material.leaf", "material", "assets/leaf.mat", {"mesh.tree"}}),
          "registers material asset with dependency");
    check(registry.validate_dependencies(), "validates resolved dependencies");
    check(!registry.register_asset({"mesh.tree", "mesh", "assets/other.mesh", {}}), "rejects duplicate ids");
    check(!registry.register_asset({"escape", "mesh", "../../outside.mesh", {}}), "rejects paths escaping project");
    check(registry.register_asset({"missing.dep", "material", "assets/missing.mat", {"no-such-asset"}}),
          "records dependency declarations for later validation");
    auto invalid = registry.validate_dependencies();
    check(!invalid && invalid.error().code == ErrorCode::not_found, "reports missing asset dependency");
    check(registry.find("mesh.tree") != nullptr, "looks up assets by stable id");
    registry.clear();
    check(registry.size() == 0, "clears registry");
}
}
int main() {
    test_scene_graph();
    test_asset_registry();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio scene and asset tests passed.\n";
    return 0;
}
