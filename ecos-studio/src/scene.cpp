#include "ecos_studio/scene.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace ecos::studio {

Matrix4 Matrix4::identity() noexcept {
    Matrix4 result;
    result.values = {1.0F, 0.0F, 0.0F, 0.0F,
                     0.0F, 1.0F, 0.0F, 0.0F,
                     0.0F, 0.0F, 1.0F, 0.0F,
                     0.0F, 0.0F, 0.0F, 1.0F};
    return result;
}

Matrix4 multiply(const Matrix4& a, const Matrix4& b) noexcept {
    Matrix4 result;
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            float value = 0.0F;
            for (std::size_t k = 0; k < 4; ++k) {
                value += a.values[row * 4 + k] * b.values[k * 4 + column];
            }
            result.values[row * 4 + column] = value;
        }
    }
    return result;
}

Matrix4 compose(const Transform& transform) noexcept {
    const float cx = std::cos(transform.rotation_euler_radians.x);
    const float sx = std::sin(transform.rotation_euler_radians.x);
    const float cy = std::cos(transform.rotation_euler_radians.y);
    const float sy = std::sin(transform.rotation_euler_radians.y);
    const float cz = std::cos(transform.rotation_euler_radians.z);
    const float sz = std::sin(transform.rotation_euler_radians.z);

    // Row-major matrix; Euler rotation order is Z * Y * X.
    Matrix4 result;
    result.values = {
        (cz * cy) * transform.scale.x,
        (cz * sy * sx - sz * cx) * transform.scale.y,
        (cz * sy * cx + sz * sx) * transform.scale.z,
        transform.position.x,
        (sz * cy) * transform.scale.x,
        (sz * sy * sx + cz * cx) * transform.scale.y,
        (sz * sy * cx - cz * sx) * transform.scale.z,
        transform.position.y,
        (-sy) * transform.scale.x,
        (cy * sx) * transform.scale.y,
        (cy * cx) * transform.scale.z,
        transform.position.z,
        0.0F, 0.0F, 0.0F, 1.0F
    };
    return result;
}

Result<NodeId> SceneGraph::create_node(std::string name, std::optional<NodeId> parent) {
    if (name.empty()) {
        return Result<NodeId>::failure({ErrorCode::invalid_argument, "Scene node name must not be empty"});
    }
    if (parent && nodes_.find(*parent) == nodes_.end()) {
        return Result<NodeId>::failure({ErrorCode::not_found, "Parent scene node does not exist"});
    }

    const NodeId id = next_id_++;
    SceneNode node;
    node.id = id;
    node.name = std::move(name);
    node.parent = parent;
    nodes_.emplace(id, std::move(node));
    if (parent) {
        nodes_.at(*parent).children.push_back(id);
    } else {
        roots_.push_back(id);
    }
    return Result<NodeId>::success(id);
}

bool SceneGraph::is_ancestor(NodeId candidate, NodeId node) const noexcept {
    auto current = find(node);
    while (current && current->parent) {
        if (*current->parent == candidate) return true;
        current = find(*current->parent);
    }
    return false;
}

void SceneGraph::detach_from_parent(SceneNode& node) noexcept {
    if (node.parent) {
        auto parent = nodes_.find(*node.parent);
        if (parent != nodes_.end()) {
            auto& children = parent->second.children;
            children.erase(std::remove(children.begin(), children.end(), node.id), children.end());
        }
    } else {
        roots_.erase(std::remove(roots_.begin(), roots_.end(), node.id), roots_.end());
    }
}

Result<void> SceneGraph::set_parent(NodeId node_id, std::optional<NodeId> parent_id) {
    auto node_it = nodes_.find(node_id);
    if (node_it == nodes_.end()) {
        return Result<void>::failure({ErrorCode::not_found, "Scene node does not exist"});
    }
    if (parent_id && nodes_.find(*parent_id) == nodes_.end()) {
        return Result<void>::failure({ErrorCode::not_found, "New parent scene node does not exist"});
    }
    if (parent_id && (*parent_id == node_id || is_ancestor(node_id, *parent_id))) {
        return Result<void>::failure({ErrorCode::invalid_argument, "Reparenting would create a scene graph cycle"});
    }
    if (node_it->second.parent == parent_id) return Result<void>::success();

    detach_from_parent(node_it->second);
    node_it->second.parent = parent_id;
    if (parent_id) nodes_.at(*parent_id).children.push_back(node_id);
    else roots_.push_back(node_id);
    return Result<void>::success();
}

Result<void> SceneGraph::set_local_transform(NodeId node_id, const Transform& transform) {
    auto* node = find(node_id);
    if (!node) return Result<void>::failure({ErrorCode::not_found, "Scene node does not exist"});
    node->local_transform = transform;
    return Result<void>::success();
}

Result<void> SceneGraph::destroy_subtree(NodeId node_id) {
    auto it = nodes_.find(node_id);
    if (it == nodes_.end()) {
        return Result<void>::failure({ErrorCode::not_found, "Scene node does not exist"});
    }
    detach_from_parent(it->second);
    std::vector<NodeId> pending{node_id};
    while (!pending.empty()) {
        const NodeId current = pending.back();
        pending.pop_back();
        const auto current_it = nodes_.find(current);
        if (current_it == nodes_.end()) continue;
        pending.insert(pending.end(), current_it->second.children.begin(), current_it->second.children.end());
        nodes_.erase(current_it);
    }
    return Result<void>::success();
}

const SceneNode* SceneGraph::find(NodeId node) const noexcept {
    const auto it = nodes_.find(node);
    return it == nodes_.end() ? nullptr : &it->second;
}

SceneNode* SceneGraph::find(NodeId node) noexcept {
    const auto it = nodes_.find(node);
    return it == nodes_.end() ? nullptr : &it->second;
}

Result<Matrix4> SceneGraph::world_matrix(NodeId node_id) const {
    const auto* node = find(node_id);
    if (!node) return Result<Matrix4>::failure({ErrorCode::not_found, "Scene node does not exist"});
    Matrix4 result = compose(node->local_transform);
    auto parent_id = node->parent;
    while (parent_id) {
        const auto* parent = find(*parent_id);
        if (!parent) return Result<Matrix4>::failure({ErrorCode::internal_error, "Scene graph contains a missing parent"});
        result = multiply(compose(parent->local_transform), result);
        parent_id = parent->parent;
    }
    return Result<Matrix4>::success(result);
}

std::size_t SceneGraph::size() const noexcept { return nodes_.size(); }
const std::vector<NodeId>& SceneGraph::roots() const noexcept { return roots_; }

} // namespace ecos::studio
