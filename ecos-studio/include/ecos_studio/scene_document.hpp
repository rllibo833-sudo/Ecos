#pragma once

#include "ecos_studio/scene_editor.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace ecos::studio {

// Owns the editor-facing scene state so mutations, selection, history and
// save/dirty tracking stay coordinated around one stable SceneGraph instance.
class SceneDocument {
public:
    explicit SceneDocument(std::string scene_id = "untitled");

    [[nodiscard]] const std::string& scene_id() const noexcept { return scene_id_; }
    [[nodiscard]] const SceneGraph& scene() const noexcept { return scene_; }
    [[nodiscard]] SceneEditorSelection& selection() noexcept { return selection_; }
    [[nodiscard]] const SceneEditorSelection& selection() const noexcept { return selection_; }
    [[nodiscard]] const SceneEditorHistory& history() const noexcept { return history_; }

    [[nodiscard]] bool is_dirty() const noexcept { return dirty_; }
    [[nodiscard]] std::uint64_t revision() const noexcept { return revision_; }
    [[nodiscard]] std::uint64_t saved_revision() const noexcept { return saved_revision_; }

    [[nodiscard]] Result<NodeId> create_node(std::string name, std::optional<NodeId> parent = std::nullopt);
    [[nodiscard]] Result<void> delete_subtree(NodeId node);
    [[nodiscard]] Result<void> rename_node(NodeId node, std::string name);
    [[nodiscard]] Result<void> set_transform(NodeId node, const Transform& transform);
    [[nodiscard]] Result<void> reparent(NodeId node, std::optional<NodeId> parent);
    [[nodiscard]] Result<void> set_mesh_asset(NodeId node, std::string asset_id);
    [[nodiscard]] Result<void> set_material_asset(NodeId node, std::string asset_id);
    [[nodiscard]] Result<void> undo();
    [[nodiscard]] Result<void> redo();

    // Serialization is in-memory; callers decide where/how to persist files.
    // A successful save advances the saved revision. A failed save leaves the
    // document dirty.
    [[nodiscard]] Result<std::string> save();
    [[nodiscard]] Result<void> load(const std::string& serialized_scene);

private:
    void mark_changed() noexcept;

    std::string scene_id_;
    SceneGraph scene_;
    SceneEditorSelection selection_;
    SceneEditorHistory history_;
    std::uint64_t revision_{0};
    std::uint64_t saved_revision_{0};
    bool dirty_{false};
};

} // namespace ecos::studio
