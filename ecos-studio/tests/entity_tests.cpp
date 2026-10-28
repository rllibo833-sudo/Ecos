#include "ecos_studio/entity.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

struct Position { float x{0.0F}; float y{0.0F}; float z{0.0F}; };
struct Name { std::string value; };

void test_entity_lifecycle_and_components() {
    using namespace ecos::studio;
    EntityRegistry world;
    const auto player = world.create();
    const auto tree = world.create();
    check(world.alive(player) && world.alive(tree), "new entities are alive");
    check(world.entity_count() == 2, "tracks entity count");

    auto& position = world.emplace<Position>(player, Position{1.0F, 2.0F, 3.0F});
    position.x = 4.0F;
    world.emplace<Name>(player, Name{"player"});
    world.emplace<Position>(tree, Position{9.0F, 0.0F, 1.0F});
    check(world.get<Position>(player) && world.get<Position>(player)->x == 4.0F,
          "stores mutable typed components");
    check(world.get<Name>(player) && world.get<Name>(player)->value == "player",
          "stores distinct component types");
    check(world.get<Name>(tree) == nullptr && !world.has<Name>(tree),
          "missing components return null safely");
    check(world.component_count<Position>() == 2, "counts components");

    const auto with_positions = world.entities_with<Position>();
    check(with_positions.size() == 2 && with_positions[0] == player && with_positions[1] == tree,
          "component query has deterministic entity order");
    check(world.remove<Name>(player) && !world.has<Name>(player),
          "removes a component");
    check(!world.remove<Name>(player), "reports absent component removal");

    check(world.destroy(player), "destroys live entity");
    check(!world.alive(player) && world.get<Position>(player) == nullptr,
          "stale entity handles cannot access components");
    check(world.component_count<Position>() == 1 && world.entity_count() == 1,
          "destroy cleans components and updates count");
    check(!world.destroy(player), "rejects repeated destruction");

    const auto replacement = world.create();
    check(replacement.index == player.index && replacement.generation != player.generation,
          "reuses slots with a new generation");
    check(world.alive(replacement) && !world.alive(player),
          "stale handle stays invalid after slot reuse");
    bool duplicate_rejected = false;
    try {
        world.emplace<Position>(replacement, Position{});
        world.emplace<Position>(replacement, Position{});
    } catch (const std::logic_error&) {
        duplicate_rejected = true;
    }
    check(duplicate_rejected, "rejects duplicate component insertion");
    bool stale_rejected = false;
    try {
        world.emplace<Name>(player, Name{"stale"});
    } catch (const std::invalid_argument&) {
        stale_rejected = true;
    }
    check(stale_rejected, "rejects component insertion on stale handle");
}
}

int main() {
    test_entity_lifecycle_and_components();
    if (failures != 0) {
        std::cerr << failures << " ECS test(s) failed.\n";
        return 1;
    }
    std::cout << "ECOS Studio entity/component tests passed.\n";
    return 0;
}
