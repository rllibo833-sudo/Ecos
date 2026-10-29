#include "ecos_studio/scene.hpp"

#include <cmath>
#include <iostream>
#include <string>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_round_trip() {
    using namespace ecos::studio;
    SceneGraph graph;
    auto root = graph.create_node("World");
    auto player = graph.create_node("Player", root.value());
    auto camera = graph.create_node("Camera", player.value());
    Transform root_transform;
    root_transform.position = {120.5F, 0.0F, -4.0F};
    check(graph.set_local_transform(root.value(), root_transform), "sets root transform");
    Transform camera_transform;
    camera_transform.position = {0.0F, 1.7F, 0.0F};
    camera_transform.rotation_euler_radians = {0.1F, 0.0F, 0.0F};
    check(graph.set_local_transform(camera.value(), camera_transform), "sets camera transform");

    auto serialized = serialize_scene("sample-world", graph);
    check(serialized, "serializes scene");
    if (!serialized) return;
    auto loaded = deserialize_scene(serialized.value());
    if (!loaded) std::cerr << "Deserialize diagnostic: " << loaded.error().message << '\n';
    check(loaded, "deserializes scene");
    if (loaded) {
        check(loaded.value().size() == 3, "round trip preserves node count");
        const auto roots = loaded.value().roots();
        check(roots.size() == 1, "round trip preserves root count");
        if (roots.empty()) return;
        const auto* loaded_root = loaded.value().find(roots.front());
        check(loaded_root && loaded_root->name == "World", "round trip preserves root name");
        if (loaded_root && !loaded_root->children.empty()) {
            const auto* loaded_player = loaded.value().find(loaded_root->children.front());
            check(loaded_player && loaded_player->name == "Player", "round trip preserves parent-child hierarchy");
            if (loaded_player && !loaded_player->children.empty()) {
                auto matrix = loaded.value().world_matrix(loaded_player->children.front());
                check(matrix, "computes loaded camera world matrix");
                if (matrix) check(std::fabs(matrix.value().values[7] - 1.7F) < 0.0001F,
                                  "round trip preserves world-space camera height");
            }
        }
    }
}
void test_preserves_ids_and_multiple_roots() {
    using namespace ecos::studio;
    SceneGraph graph;
    auto first = graph.create_node("First");
    auto removed = graph.create_node("Removed");
    auto third = graph.create_node("Third");
    auto child = graph.create_node("Child", third.value());
    check(first && removed && third && child, "creates nodes for stable-id round trip");
    if (!first || !removed || !third || !child) return;
    check(graph.destroy_subtree(removed.value()), "creates an ID gap");
    auto serialized = serialize_scene("stable-ids", graph);
    check(serialized, "serializes graph with ID gap and multiple roots");
    if (!serialized) return;
    auto loaded = deserialize_scene(serialized.value());
    check(loaded, "loads graph with ID gap and multiple roots");
    if (!loaded) return;
    check(loaded.value().find(first.value()) != nullptr, "preserves first root ID");
    check(loaded.value().find(third.value()) != nullptr, "preserves root ID after gap");
    check(loaded.value().find(child.value()) != nullptr, "preserves descendant ID");
    check(loaded.value().roots().size() == 2, "preserves multiple roots");
    auto next = loaded.value().create_node("Next");
    check(next && next.value() > child.value(), "new IDs do not collide after loading sparse IDs");
}


void test_large_deep_scene_round_trip() {
    using namespace ecos::studio;
    SceneGraph graph;
    auto parent = graph.create_node("Node-0");
    check(parent, "creates root for deep scene");
    if (!parent) return;
    for (int i = 1; i < 1500; ++i) {
        auto child = graph.create_node("Node-" + std::to_string(i), parent.value());
        check(child, "creates child in deep scene");
        if (!child) return;
        parent = child;
    }
    auto serialized = serialize_scene("deep-scene", graph);
    check(serialized, "serializes deep scene");
    if (!serialized) return;
    auto loaded = deserialize_scene(serialized.value());
    check(loaded, "deserializes deep scene without quadratic hierarchy validation");
    if (loaded) {
        check(loaded.value().size() == 1500, "deep round trip preserves all nodes");
        check(loaded.value().find(1500) != nullptr, "deep round trip preserves deepest node ID");
    }
}

void test_mesh_asset_reference_round_trip_and_v1_compatibility() {
    using namespace ecos::studio;
    SceneGraph graph;
    auto root = graph.create_node("Root");
    auto mesh_node = graph.create_node("Crate", root.value());
    check(root && mesh_node, "creates mesh asset scene nodes");
    if (!root || !mesh_node) return;
    check(graph.set_mesh_asset(mesh_node.value(), "crate.mesh"), "assigns mesh asset reference");
    auto v2_serialized = serialize_scene("asset-scene-v2", graph);
    check(v2_serialized && v2_serialized.value().find("format_version=2\n") == 0,
          "uses scene format v2 when only mesh refs exist");
    if (v2_serialized) {
        auto v2 = deserialize_scene(v2_serialized.value());
        check(v2 && v2.value().find(mesh_node.value())->mesh_asset_id == "crate.mesh",
              "round trips v2 mesh asset references");
        check(v2 && v2.value().find(mesh_node.value())->material_asset_id.empty(),
              "v2 scene has no material reference");
    }
    check(graph.set_material_asset(mesh_node.value(), "painted.mtl"), "assigns material asset reference");
    auto serialized = serialize_scene("asset-scene", graph);
    check(serialized && serialized.value().find("format_version=3\n") == 0,
          "uses scene format v3 when material refs exist");
    if (!serialized) return;
    auto loaded = deserialize_scene(serialized.value());
    check(loaded, "loads scene format v3");
    if (loaded) {
        const auto* node = loaded.value().find(mesh_node.value());
        check(node && node->mesh_asset_id == "crate.mesh", "round trip preserves mesh asset ID");
        check(node && node->material_asset_id == "painted.mtl", "round trip preserves material asset ID");
    }
    auto legacy = deserialize_scene("format_version=1\nscene_id=legacy\nnode=1|-|Legacy|0,0,0|0,0,0|1,1,1\n");
    check(legacy && legacy.value().find(1)->mesh_asset_id.empty(), "v1 scene remains compatible");
    check(!graph.set_mesh_asset(mesh_node.value(), "bad|asset"), "rejects delimiters in mesh asset ID");
    check(!graph.set_material_asset(mesh_node.value(), "bad|material"), "rejects delimiters in material asset ID");
}
void test_rejects_bad_documents() {
    using namespace ecos::studio;
    check(!deserialize_scene("scene_id=x\nnode=1|-|World|0,0,0|0,0,0|1,1,1\n"),
          "requires explicit schema version");
    check(!deserialize_scene("format_version=9\nscene_id=x\nnode=1|-|World|0,0,0|0,0,0|1,1,1\n"),
          "rejects unsupported schema version");
    check(!deserialize_scene("format_version=1\nscene_id=x\nnode=1|99|Child|0,0,0|0,0,0|1,1,1\n"),
          "rejects missing parent");
    check(!deserialize_scene("format_version=1\nscene_id=x\nnode=1|2|A|0,0,0|0,0,0|1,1,1\nnode=2|1|B|0,0,0|0,0,0|1,1,1\n"),
          "rejects parent cycles");
    SceneGraph graph;
    check(graph.create_node("bad|name"), "runtime accepts general names");
    check(!serialize_scene("scene", graph), "serializer rejects unescaped delimiters rather than corrupting file");
}
}
int main() {
    test_round_trip();
    test_preserves_ids_and_multiple_roots();
    test_large_deep_scene_round_trip();
    test_rejects_bad_documents();
    test_mesh_asset_reference_round_trip_and_v1_compatibility();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio scene serialization tests passed.\n";
    return 0;
}
