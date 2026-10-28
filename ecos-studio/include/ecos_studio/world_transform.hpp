#pragma once

#include "ecos_studio/entity.hpp"
#include "ecos_studio/result.hpp"
#include "ecos_studio/scene.hpp"

#include <cstddef>

namespace ecos::studio {

struct SceneNodeComponent;

// Cached hierarchical transform for an entity. Matrix convention matches SceneGraph:
// parent world matrix multiplied by the node's local transform matrix.
struct WorldTransformComponent {
    Matrix4 matrix{Matrix4::identity()};
};

// Recomputes world transforms for every entity with SceneNodeComponent.
// Updates are transactional: if a hierarchy is invalid, no cached matrices are changed.
class WorldTransformSystem {
public:
    [[nodiscard]] static Result<std::size_t> update(EntityRegistry& entities);
};

} // namespace ecos::studio
