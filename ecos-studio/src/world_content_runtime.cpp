#include "ecos_studio/world_content_runtime.hpp"

#include <utility>
#include <exception>

namespace ecos::studio {

Result<std::unique_ptr<WorldContentRuntime>> WorldContentRuntime::create(
    WorldManifest manifest, JobSystem& jobs, std::size_t max_loaded_cells) {
    auto world = manifest.create_runtime(max_loaded_cells);
    if (!world) return Result<std::unique_ptr<WorldContentRuntime>>::failure(world.error());
    return Result<std::unique_ptr<WorldContentRuntime>>::success(
        std::unique_ptr<WorldContentRuntime>(
            new WorldContentRuntime(std::move(manifest), std::move(world.value()), jobs)));
}

WorldContentRuntime::WorldContentRuntime(WorldManifest manifest, WorldRuntime world, JobSystem& jobs)
    : manifest_(std::move(manifest)),
      world_(std::move(world)),
      streaming_(world_, jobs,
          [this](const WorldCell& cell) { return load_cell(cell); },
          [this](const WorldCell& cell) { return unload_cell(cell); }) {}

Result<void> WorldContentRuntime::load_cell(const WorldCell& cell) {
    auto content = load_world_cell_content(manifest_, cell);
    if (!content) return Result<void>::failure(content.error());
    std::lock_guard lock(content_mutex_);
    const auto inserted = resident_content_.emplace(cell.id, std::move(content.value()));
    if (!inserted.second) {
        return Result<void>::failure({ErrorCode::invalid_state,
            "World cell content is already resident: " + cell.id});
    }
    return Result<void>::success();
}

Result<void> WorldContentRuntime::unload_cell(const WorldCell& cell) {
    std::lock_guard lock(content_mutex_);
    const auto content = resident_content_.find(cell.id);
    if (content == resident_content_.end()) {
        return Result<void>::failure({ErrorCode::invalid_state,
            "Cannot unload world cell without resident scene content: " + cell.id});
    }
    const auto entity_group = cell_entities_.find(cell.id);
    if (entity_group != cell_entities_.end()) {
        for (const auto& [node_id, entity] : entity_group->second) {
            (void)node_id;
            (void)entities_.destroy(entity);
        }
        cell_entities_.erase(entity_group);
    }
    resident_content_.erase(content);
    return Result<void>::success();
}

Result<void> WorldContentRuntime::materialize_cell_entities(const std::string& cell_id) {
    if (cell_entities_.find(cell_id) != cell_entities_.end()) return Result<void>::success();
    auto scene = scene_snapshot(cell_id);
    if (!scene) return Result<void>::failure(scene.error());
    auto& group = cell_entities_[cell_id];
    try {
        for (const auto& node : scene.value().nodes()) {
            const auto entity = entities_.create();
            try {
                entities_.emplace<WorldCellEntityComponent>(
                    entity, WorldCellEntityComponent{cell_id, node.id});
                entities_.emplace<SceneNodeComponent>(
                    entity, SceneNodeComponent{node.name, node.local_transform, node.parent});
                group.emplace(node.id, entity);
            } catch (...) {
                (void)entities_.destroy(entity);
                throw;
            }
        }
    } catch (const std::exception& exception) {
        for (const auto& [node_id, entity] : group) {
            (void)node_id;
            (void)entities_.destroy(entity);
        }
        cell_entities_.erase(cell_id);
        return Result<void>::failure({ErrorCode::internal_error,
            "Could not materialize entities for world cell '" + cell_id + "': " + exception.what()});
    } catch (...) {
        for (const auto& [node_id, entity] : group) {
            (void)node_id;
            (void)entities_.destroy(entity);
        }
        cell_entities_.erase(cell_id);
        return Result<void>::failure({ErrorCode::internal_error,
            "Could not materialize entities for world cell '" + cell_id + "'"});
    }
    return Result<void>::success();
}

Result<StreamingUpdateReport> WorldContentRuntime::update(
    CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius) {
    auto updated = streaming_.update(center, load_radius, unload_radius);
    if (!updated) return updated;
    for (const auto& cell : manifest_.cells) {
        const auto* status = world_.find(cell.id);
        if (status && status->state == CellState::loaded &&
            cell_entities_.find(cell.id) == cell_entities_.end()) {
            auto materialized = materialize_cell_entities(cell.id);
            if (!materialized) return Result<StreamingUpdateReport>::failure(materialized.error());
        }
    }
    return updated;
}

Result<void> WorldContentRuntime::retry_failed_cell(const std::string& id) {
    return streaming_.retry_failed_cell(id);
}

Result<SceneGraph> WorldContentRuntime::scene_snapshot(const std::string& cell_id) const {
    const auto* status = world_.find(cell_id);
    if (!status) return Result<SceneGraph>::failure(
        {ErrorCode::not_found, "World cell does not exist: " + cell_id});
    if (status->state != CellState::loaded) return Result<SceneGraph>::failure(
        {ErrorCode::invalid_state, "World cell scene is not loaded: " + cell_id});
    std::lock_guard lock(content_mutex_);
    const auto found = resident_content_.find(cell_id);
    if (found == resident_content_.end()) return Result<SceneGraph>::failure(
        {ErrorCode::invalid_state, "Loaded world cell has no resident scene: " + cell_id});
    return Result<SceneGraph>::success(found->second.scene);
}

std::size_t WorldContentRuntime::resident_content_count() const {
    std::lock_guard lock(content_mutex_);
    return resident_content_.size();
}
std::size_t WorldContentRuntime::entity_count() const noexcept { return entities_.entity_count(); }
std::optional<Entity> WorldContentRuntime::entity_for_node(
    const std::string& cell_id, NodeId node_id) const noexcept {
    const auto cell = cell_entities_.find(cell_id);
    if (cell == cell_entities_.end()) return std::nullopt;
    const auto node = cell->second.find(node_id);
    if (node == cell->second.end()) return std::nullopt;
    return node->second;
}
const EntityRegistry& WorldContentRuntime::entities() const noexcept { return entities_; }

std::size_t WorldContentRuntime::in_flight_count() const noexcept { return streaming_.in_flight_count(); }
bool WorldContentRuntime::idle() const noexcept { return streaming_.idle(); }
const WorldManifest& WorldContentRuntime::manifest() const noexcept { return manifest_; }
WorldRuntime& WorldContentRuntime::world() noexcept { return world_; }
const WorldRuntime& WorldContentRuntime::world() const noexcept { return world_; }

} // namespace ecos::studio
