#include "ecos_studio/scene_document.hpp"

#include <sstream>
#include <utility>

namespace ecos::studio {

SceneDocument::SceneDocument(std::string scene_id)
    : scene_id_(std::move(scene_id)), selection_(scene_), history_(scene_) {}

void SceneDocument::mark_changed() noexcept {
    if (revision_ != UINT64_MAX) ++revision_;
    dirty_ = true;
}

Result<NodeId> SceneDocument::create_node(std::string name, std::optional<NodeId> parent) {
    auto result = history_.create_node(std::move(name), parent);
    if (result) mark_changed();
    return result;
}

Result<void> SceneDocument::delete_subtree(NodeId node) {
    auto result = history_.delete_subtree(node);
    if (result) {
        selection_.prune_deleted();
        mark_changed();
    }
    return result;
}

Result<void> SceneDocument::rename_node(NodeId node, std::string name) {
    auto result = history_.rename_node(node, std::move(name));
    if (result) mark_changed();
    return result;
}

Result<void> SceneDocument::set_transform(NodeId node, const Transform& transform) {
    auto result = history_.set_transform(node, transform);
    if (result) mark_changed();
    return result;
}

Result<void> SceneDocument::reparent(NodeId node, std::optional<NodeId> parent) {
    auto result = history_.reparent(node, parent);
    if (result) mark_changed();
    return result;
}

Result<void> SceneDocument::set_mesh_asset(NodeId node, std::string asset_id) {
    auto result = history_.set_mesh_asset(node, std::move(asset_id));
    if (result) mark_changed();
    return result;
}

Result<void> SceneDocument::set_material_asset(NodeId node, std::string asset_id) {
    auto result = history_.set_material_asset(node, std::move(asset_id));
    if (result) mark_changed();
    return result;
}

Result<void> SceneDocument::undo() {
    auto result = history_.undo();
    if (result) {
        selection_.prune_deleted();
        mark_changed();
    }
    return result;
}

Result<void> SceneDocument::redo() {
    auto result = history_.redo();
    if (result) {
        selection_.prune_deleted();
        mark_changed();
    }
    return result;
}

Result<std::string> SceneDocument::save() {
    auto serialized = serialize_scene(scene_id_, scene_);
    if (!serialized) return serialized;
    saved_revision_ = revision_;
    dirty_ = false;
    return serialized;
}

Result<void> SceneDocument::load(const std::string& serialized_scene) {
    auto loaded = deserialize_scene(serialized_scene);
    if (!loaded) return Result<void>::failure(loaded.error());

    std::string loaded_scene_id;
    std::istringstream input(serialized_scene);
    std::string line;
    while (std::getline(input, line)) {
        constexpr char prefix[] = "scene_id=";
        if (line.rfind(prefix, 0) == 0) {
            loaded_scene_id = line.substr(sizeof(prefix) - 1);
            break;
        }
    }
    if (loaded_scene_id.empty())
        return Result<void>::failure({ErrorCode::invalid_argument, "Serialized scene is missing scene_id"});

    // Keep scene_ itself at a stable address because selection/history retain
    // references to it. Replace its value only after parsing fully succeeds.
    history_.clear();
    selection_.clear();
    scene_ = std::move(loaded).value();
    scene_id_ = std::move(loaded_scene_id);
    revision_ = 0;
    saved_revision_ = 0;
    dirty_ = false;
    return Result<void>::success();
}

} // namespace ecos::studio
