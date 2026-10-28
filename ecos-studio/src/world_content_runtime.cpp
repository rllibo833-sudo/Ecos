#include "ecos_studio/world_content_runtime.hpp"

#include <utility>

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
    const auto erased = resident_content_.erase(cell.id);
    if (erased == 0) {
        return Result<void>::failure({ErrorCode::invalid_state,
            "Cannot unload world cell without resident scene content: " + cell.id});
    }
    return Result<void>::success();
}

Result<StreamingUpdateReport> WorldContentRuntime::update(
    CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius) {
    return streaming_.update(center, load_radius, unload_radius);
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

std::size_t WorldContentRuntime::in_flight_count() const noexcept { return streaming_.in_flight_count(); }
bool WorldContentRuntime::idle() const noexcept { return streaming_.idle(); }
const WorldManifest& WorldContentRuntime::manifest() const noexcept { return manifest_; }
WorldRuntime& WorldContentRuntime::world() noexcept { return world_; }
const WorldRuntime& WorldContentRuntime::world() const noexcept { return world_; }

} // namespace ecos::studio
