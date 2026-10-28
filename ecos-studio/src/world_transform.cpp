#include "ecos_studio/world_transform.hpp"

#include "ecos_studio/world_content_runtime.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

namespace ecos::studio {

Result<std::size_t> WorldTransformSystem::update(EntityRegistry& entities) {
    const auto nodes = entities.entities_with<SceneNodeComponent>();
    std::unordered_map<Entity, unsigned char, EntityHash> state;
    std::unordered_map<Entity, Matrix4, EntityHash> computed;
    state.reserve(nodes.size());
    computed.reserve(nodes.size());

    std::function<Result<Matrix4>(Entity)> resolve = [&](Entity entity) -> Result<Matrix4> {
        const auto cached = computed.find(entity);
        if (cached != computed.end()) return Result<Matrix4>::success(cached->second);
        const auto* node = entities.get<SceneNodeComponent>(entity);
        if (!node) return Result<Matrix4>::failure(
            {ErrorCode::invalid_state, "Transform hierarchy references an entity without a scene node"});
        auto& mark = state[entity];
        if (mark == 1) return Result<Matrix4>::failure(
            {ErrorCode::invalid_argument, "Scene entity hierarchy contains a cycle"});
        if (mark == 2) {
            const auto found = computed.find(entity);
            if (found != computed.end()) return Result<Matrix4>::success(found->second);
        }
        mark = 1;
        Matrix4 world = compose(node->local_transform);
        if (node->parent_entity) {
            if (!entities.alive(*node->parent_entity))
                return Result<Matrix4>::failure(
                    {ErrorCode::invalid_state, "Scene entity hierarchy references a stale parent"});
            auto parent = resolve(*node->parent_entity);
            if (!parent) return parent;
            world = multiply(parent.value(), world);
        }
        mark = 2;
        computed.emplace(entity, world);
        return Result<Matrix4>::success(world);
    };

    for (const auto entity : nodes) {
        auto result = resolve(entity);
        if (!result) return Result<std::size_t>::failure(result.error());
    }
    for (const auto entity : nodes) {
        const auto found = computed.find(entity);
        if (found == computed.end()) continue;
        auto* transform = entities.get<WorldTransformComponent>(entity);
        if (transform) transform->matrix = found->second;
        else entities.emplace<WorldTransformComponent>(entity, WorldTransformComponent{found->second});
    }
    return Result<std::size_t>::success(nodes.size());
}

} // namespace ecos::studio
