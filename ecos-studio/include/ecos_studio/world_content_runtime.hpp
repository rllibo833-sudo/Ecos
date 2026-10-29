#pragma once

#include "ecos_studio/entity.hpp"
#include "ecos_studio/job_system.hpp"
#include "ecos_studio/world_content.hpp"
#include "ecos_studio/world_streaming.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace ecos::studio {

struct WorldCellEntityComponent {
    std::string cell_id;
    NodeId scene_node_id{0};
};

struct SceneNodeComponent {
    std::string name;
    std::string mesh_asset_id;
    std::string material_asset_id;
    Transform local_transform{};
    std::optional<NodeId> parent;
    std::optional<Entity> parent_entity;
};

// Owns the manifest-backed world lifecycle and keeps parsed scene graphs resident
// exactly while their cells are loaded. Worker threads perform file IO/parsing;
// scene snapshots and cell eviction are synchronized. Call update() and ECS queries\n// from the owning runtime thread; worker threads only load and parse cell files.
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
    [[nodiscard]] std::size_t entity_count() const noexcept;
    [[nodiscard]] std::optional<Entity> entity_for_node(const std::string& cell_id, NodeId node_id) const noexcept;
    [[nodiscard]] const EntityRegistry& entities() const noexcept;
    [[nodiscard]] std::size_t in_flight_count() const noexcept;
    [[nodiscard]] bool idle() const noexcept;
    [[nodiscard]] const WorldManifest& manifest() const noexcept;
    [[nodiscard]] const WorldRuntime& world() const noexcept;

private:
    WorldContentRuntime(WorldManifest manifest, WorldRuntime world, JobSystem& jobs);
    [[nodiscard]] Result<void> load_cell(const WorldCell& cell);
    [[nodiscard]] Result<void> unload_cell(const WorldCell& cell);
    [[nodiscard]] Result<void> materialize_cell_entities(const std::string& cell_id);

    WorldManifest manifest_;
    WorldRuntime world_;
    mutable std::mutex content_mutex_;
    std::unordered_map<std::string, WorldCellContent> resident_content_;
    EntityRegistry entities_;
    std::unordered_map<std::string, std::unordered_map<NodeId, Entity>> cell_entities_;
    // Declared last so it drains workers before content/world members are destroyed.
    WorldStreamingService streaming_;
};

} // namespace ecos::studio
