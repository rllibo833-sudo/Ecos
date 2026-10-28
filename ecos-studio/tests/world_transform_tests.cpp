#include "ecos_studio/world_transform.hpp"
#include "ecos_studio/world_content_runtime.hpp"

#include <cmath>
#include <iostream>

namespace {
int failures = 0;
void check(bool value, const char* message) {
    if (!value) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool near(float a, float b) { return std::abs(a - b) < 1.0e-4F; }

void test_parent_child_and_live_updates() {
    using namespace ecos::studio;
    EntityRegistry entities;
    const auto parent = entities.create();
    const auto child = entities.create();
    Transform parent_transform;
    parent_transform.position = {10.0F, 2.0F, 0.0F};
    Transform child_transform;
    child_transform.position = {3.0F, 4.0F, 1.0F};
    entities.emplace<SceneNodeComponent>(parent, SceneNodeComponent{"Parent", "", parent_transform, {}, {}});
    entities.emplace<SceneNodeComponent>(child, SceneNodeComponent{"Child", "", child_transform, {}, parent});

    auto updated = WorldTransformSystem::update(entities);
    check(updated && updated.value() == 2, "updates every scene entity transform");
    const auto* world_parent = entities.get<WorldTransformComponent>(parent);
    const auto* world_child = entities.get<WorldTransformComponent>(child);
    check(world_parent && near(world_parent->matrix.values[3], 10.0F), "root world matrix includes local translation");
    check(world_child && near(world_child->matrix.values[3], 13.0F) &&
          near(world_child->matrix.values[7], 6.0F) && near(world_child->matrix.values[11], 1.0F),
          "child world matrix composes parent and local translation");

    entities.get<SceneNodeComponent>(parent)->local_transform.position.x = -5.0F;
    updated = WorldTransformSystem::update(entities);
    world_child = entities.get<WorldTransformComponent>(child);
    check(updated && world_child && near(world_child->matrix.values[3], -2.0F),
          "subsequent updates refresh cached matrices after local edits");
}

void test_invalid_hierarchy_does_not_partially_update() {
    using namespace ecos::studio;
    EntityRegistry entities;
    const auto a = entities.create();
    const auto b = entities.create();
    entities.emplace<SceneNodeComponent>(a, SceneNodeComponent{"A", "", {}, {}, b});
    entities.emplace<SceneNodeComponent>(b, SceneNodeComponent{"B", "", {}, {}, a});
    auto updated = WorldTransformSystem::update(entities);
    check(!updated, "rejects cyclic entity hierarchy");
    check(!entities.has<WorldTransformComponent>(a) && !entities.has<WorldTransformComponent>(b),
          "does not publish partial world matrices when hierarchy validation fails");

    EntityRegistry stale_entities;
    const auto child = stale_entities.create();
    const auto stale_parent = stale_entities.create();
    check(stale_entities.destroy(stale_parent), "fixture destroys parent before transform update");
    stale_entities.emplace<SceneNodeComponent>(child, SceneNodeComponent{"Orphan", "", {}, {}, stale_parent});
    check(!WorldTransformSystem::update(stale_entities), "rejects stale parent entity handles");
}

void test_missing_scene_node_parent() {
    using namespace ecos::studio;
    EntityRegistry entities;
    const auto parent = entities.create();
    const auto child = entities.create();
    entities.emplace<SceneNodeComponent>(child, SceneNodeComponent{"Child", "", {}, {}, parent});
    check(!WorldTransformSystem::update(entities), "rejects parent entities without scene-node components");
}
}

int main() {
    test_parent_child_and_live_updates();
    test_invalid_hierarchy_does_not_partially_update();
    test_missing_scene_node_parent();
    if (failures) { std::cerr << failures << " world transform test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio world transform tests passed.\n";
    return 0;
}
