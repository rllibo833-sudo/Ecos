#include "ecos_studio/entity.hpp"

#include <limits>

namespace ecos::studio {

Entity EntityRegistry::create() {
    if (!free_indices_.empty()) {
        const auto index = free_indices_.back();
        free_indices_.pop_back();
        alive_[index] = 1;
        ++entity_count_;
        return Entity{index, generations_[index]};
    }

    if (generations_.size() >= static_cast<std::size_t>(Entity::invalid_index)) {
        throw std::overflow_error("Entity index capacity exhausted");
    }

    const auto index = static_cast<std::uint32_t>(generations_.size());
    generations_.push_back(1);
    alive_.push_back(1);
    ++entity_count_;
    return Entity{index, 1};
}

bool EntityRegistry::destroy(Entity entity) noexcept {
    if (!alive(entity)) return false;

    for (auto& [type, storage] : pools_) {
        (void)type;
        storage->erase(entity);
    }

    alive_[entity.index] = 0;
    auto& generation = generations_[entity.index];
    ++generation;
    if (generation == 0) generation = 1;
    free_indices_.push_back(entity.index);
    --entity_count_;
    return true;
}

bool EntityRegistry::alive(Entity entity) const noexcept {
    return entity.index < generations_.size()
        && alive_[entity.index] != 0
        && generations_[entity.index] == entity.generation;
}

std::size_t EntityRegistry::entity_count() const noexcept {
    return entity_count_;
}

} // namespace ecos::studio
