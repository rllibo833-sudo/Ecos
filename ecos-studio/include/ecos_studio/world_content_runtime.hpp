#pragma once

#include "ecos_studio/job_system.hpp"
#include "ecos_studio/world_content.hpp"
#include "ecos_studio/world_streaming.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ecos::studio {

// Owns the manifest-backed world lifecycle and keeps parsed scene graphs resident
// exactly while their cells are loaded. Worker threads perform file IO/parsing;
// scene snapshots and cell eviction are synchronized for safe editor/runtime use.
class WorldContentRuntime {
public:
    [[nodiscard]] static Result<std::unique_ptr<WorldContentRuntime>> create(
        WorldManifest manifest, JobSystem& jobs, std::size_t max_loaded_cells = 64);

    WorldContentRuntime(const WorldContentRuntime&) = delete;
    WorldContentRuntime& operator=(const WorldContentRuntime&) = delete;

    [[nodiscard]] Result<StreamingUpdateReport> update(
        CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius);
    [[nodiscard]] Result<void> retry_failed_cell(const std::string& id);
    [[nodiscard]] Result<SceneGraph> scene_snapshot(const std::string& cell_id) const;
    [[nodiscard]] std::size_t resident_content_count() const;
    [[nodiscard]] std::size_t in_flight_count() const noexcept;
    [[nodiscard]] bool idle() const noexcept;
    [[nodiscard]] const WorldManifest& manifest() const noexcept;
    [[nodiscard]] WorldRuntime& world() noexcept;
    [[nodiscard]] const WorldRuntime& world() const noexcept;

private:
    WorldContentRuntime(WorldManifest manifest, WorldRuntime world, JobSystem& jobs);
    [[nodiscard]] Result<void> load_cell(const WorldCell& cell);
    [[nodiscard]] Result<void> unload_cell(const WorldCell& cell);

    WorldManifest manifest_;
    WorldRuntime world_;
    mutable std::mutex content_mutex_;
    std::unordered_map<std::string, WorldCellContent> resident_content_;
    // Declared last so it drains workers before content/world members are destroyed.
    WorldStreamingService streaming_;
};

} // namespace ecos::studio
