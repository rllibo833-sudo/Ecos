#include "ecos_studio/scene.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <sstream>
#include <tuple>
#include <unordered_set>
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
            for (std::size_t k = 0; k < 4; ++k) value += a.values[row * 4 + k] * b.values[k * 4 + column];
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
    if (name.empty()) return Result<NodeId>::failure({ErrorCode::invalid_argument, "Scene node name must not be empty"});
    if (parent && nodes_.find(*parent) == nodes_.end())
        return Result<NodeId>::failure({ErrorCode::not_found, "Parent scene node does not exist"});
    const NodeId id = next_id_++;
    SceneNode node;
    node.id = id;
    node.name = std::move(name);
    node.parent = parent;
    nodes_.emplace(id, std::move(node));
    if (parent) nodes_.at(*parent).children.push_back(id);
    else roots_.push_back(id);
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
    if (node_it == nodes_.end()) return Result<void>::failure({ErrorCode::not_found, "Scene node does not exist"});
    if (parent_id && nodes_.find(*parent_id) == nodes_.end())
        return Result<void>::failure({ErrorCode::not_found, "New parent scene node does not exist"});
    if (parent_id && (*parent_id == node_id || is_ancestor(node_id, *parent_id)))
        return Result<void>::failure({ErrorCode::invalid_argument, "Reparenting would create a scene graph cycle"});
    if (node_it->second.parent == parent_id) return Result<void>::success();
    detach_from_parent(node_it->second);
    node_it->second.parent = parent_id;
    if (parent_id) nodes_.at(*parent_id).children.push_back(node_id);
    else roots_.push_back(node_id);
    return Result<void>::success();
}

Result<void> SceneGraph::rename_node(NodeId node_id, std::string name) {
    if (name.empty()) return Result<void>::failure({ErrorCode::invalid_argument, "Scene node name must not be empty"});
    auto* node = find(node_id);
    if (!node) return Result<void>::failure({ErrorCode::not_found, "Scene node does not exist"});
    node->name = std::move(name);
    return Result<void>::success();
}

Result<void> SceneGraph::restore_subtree(const std::vector<SceneNode>& snapshot) {
    if (snapshot.empty())
        return Result<void>::failure({ErrorCode::invalid_argument, "Cannot restore an empty scene subtree"});

    std::unordered_map<NodeId, SceneNode> staged;
    staged.reserve(snapshot.size());
    NodeId maximum_id = 0;
    for (const auto& original : snapshot) {
        if (original.id == 0 || nodes_.contains(original.id) ||
            !staged.emplace(original.id, original).second)
            return Result<void>::failure({ErrorCode::invalid_argument, "Scene subtree contains an invalid or duplicate node id"});
        staged.at(original.id).children.clear();
        maximum_id = std::max(maximum_id, original.id);
    }
    if (maximum_id == std::numeric_limits<NodeId>::max())
        return Result<void>::failure({ErrorCode::invalid_argument, "Scene node id space is exhausted"});

    std::size_t roots = 0;
    NodeId root_id = 0;
    for (const auto& [id, node] : staged) {
        if (!node.parent || !staged.contains(*node.parent)) {
            ++roots;
            root_id = id;
            if (node.parent && !nodes_.contains(*node.parent))
                return Result<void>::failure({ErrorCode::not_found, "Scene subtree parent does not exist"});
        }
    }

    // Each node has at most one parent, so color-marked parent walks validate
    // the entire staged hierarchy in linear time instead of re-walking every
    // ancestor chain for every node.
    std::unordered_map<NodeId, unsigned char> visit_state;
    visit_state.reserve(staged.size());
    for (const auto& [id, node] : staged) {
        (void)node;
        if (visit_state[id] == 2) continue;
        std::vector<NodeId> path;
        NodeId current = id;
        while (staged.contains(current)) {
            const auto state = visit_state[current];
            if (state == 2) break;
            if (state == 1)
                return Result<void>::failure({ErrorCode::invalid_argument, "Scene subtree contains a hierarchy cycle"});
            visit_state[current] = 1;
            path.push_back(current);
            const auto& current_node = staged.at(current);
            if (!current_node.parent || !staged.contains(*current_node.parent)) break;
            current = *current_node.parent;
        }
        for (const auto visited : path) visit_state[visited] = 2;
    }
    if (roots != 1)
        return Result<void>::failure({ErrorCode::invalid_argument, "Scene subtree must have exactly one root"});

    for (const auto& [id, node] : staged) {
        nodes_.emplace(id, node);
    }
    for (const auto& [id, node] : staged) {
        if (node.parent && staged.contains(*node.parent))
            nodes_.at(*node.parent).children.push_back(id);
    }
    const auto& root = nodes_.at(root_id);
    if (root.parent) nodes_.at(*root.parent).children.push_back(root_id);
    else roots_.push_back(root_id);
    next_id_ = std::max(next_id_, maximum_id + 1);
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
    if (it == nodes_.end()) return Result<void>::failure({ErrorCode::not_found, "Scene node does not exist"});
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

std::vector<SceneNode> SceneGraph::nodes() const {
    std::vector<SceneNode> result;
    result.reserve(nodes_.size());
    for (const auto& [id, node] : nodes_) { (void)id; result.push_back(node); }
    std::sort(result.begin(), result.end(), [](const SceneNode& a, const SceneNode& b) { return a.id < b.id; });
    return result;
}
std::size_t SceneGraph::size() const noexcept { return nodes_.size(); }
const std::vector<NodeId>& SceneGraph::roots() const noexcept { return roots_; }

namespace {
std::vector<std::string> split_scene_fields(const std::string& value, char delimiter) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const auto pos = value.find(delimiter, start);
        fields.push_back(value.substr(start, pos == std::string::npos ? pos : pos - start));
        if (pos == std::string::npos) break;
        start = pos + 1;
    }
    return fields;
}
bool valid_token(const std::string& value) {
    return !value.empty() && value.find_first_of("|\r\n") == std::string::npos;
}
bool parse_u64(const std::string& text, std::uint64_t& value) {
    if (text.empty()) return false;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size();
}
bool parse_float(const std::string& text, float& value) {
    if (text.empty()) return false;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() && std::isfinite(value);
}
bool parse_vec3(const std::string& text, Vec3& value) {
    const auto parts = split_scene_fields(text, ',');
    return parts.size() == 3 && parse_float(parts[0], value.x) &&
        parse_float(parts[1], value.y) && parse_float(parts[2], value.z);
}
std::string vec3_text(const Vec3& value) {
    std::ostringstream out;
    out.precision(std::numeric_limits<float>::max_digits10);
    out << value.x << ',' << value.y << ',' << value.z;
    return out.str();
}
struct PendingNode {
    std::uint64_t source_id{};
    std::optional<std::uint64_t> source_parent;
    std::string name;
    Transform transform;
};
}

Result<std::string> serialize_scene(const std::string& scene_id, const SceneGraph& graph) {
    if (!valid_token(scene_id)) return Result<std::string>::failure({ErrorCode::invalid_argument, "Scene id must be non-empty and cannot contain pipes or newlines"});
    std::ostringstream out;
    out << "format_version=1\nscene_id=" << scene_id << '\n';
    for (const auto& node : graph.nodes()) {
        if (!valid_token(node.name))
            return Result<std::string>::failure({ErrorCode::invalid_argument, "Node names cannot contain pipes or newlines in scene format v1"});
        out << "node=" << node.id << '|';
        if (node.parent) out << *node.parent;
        else out << '-';
        out << '|' << node.name << '|'
            << vec3_text(node.local_transform.position) << '|'
            << vec3_text(node.local_transform.rotation_euler_radians) << '|'
            << vec3_text(node.local_transform.scale) << '\n';
    }
    return Result<std::string>::success(out.str());
}

Result<SceneGraph> deserialize_scene(const std::string& text) {
    bool version_seen = false;
    bool id_seen = false;
    bool has_version_one = false;
    std::vector<PendingNode> pending;
    std::unordered_set<std::uint64_t> seen_source_ids;
    std::string line;
    std::istringstream input(text);
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty() || line[0] == '#') continue;
        const auto separator = line.find('=');
        if (separator == std::string::npos)
            return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene line " + std::to_string(line_number) + " must be key=value"});
        const auto key = line.substr(0, separator);
        const auto value = line.substr(separator + 1);
        if (key == "format_version") {
            if (version_seen) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Duplicate scene format_version"});
            version_seen = true;
            has_version_one = value == "1";
            if (!has_version_one) return Result<SceneGraph>::failure({ErrorCode::unsupported, "Unsupported scene format version"});
        } else if (key == "scene_id") {
            if (id_seen) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Duplicate scene_id"});
            id_seen = true;
            if (!valid_token(value)) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Invalid scene_id"});
        } else if (key == "node") {
            const auto fields = split_scene_fields(value, '|');
            if (fields.size() != 6) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene node record must contain six fields"});
            PendingNode node;
            if (!parse_u64(fields[0], node.source_id) || node.source_id == 0)
                return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Invalid scene node id"});
            if (fields[1] != "-") {
                std::uint64_t parent{};
                if (!parse_u64(fields[1], parent) || parent == 0)
                    return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Invalid scene parent id"});
                node.source_parent = parent;
            }
            if (!valid_token(fields[2]) || !parse_vec3(fields[3], node.transform.position) ||
                !parse_vec3(fields[4], node.transform.rotation_euler_radians) ||
                !parse_vec3(fields[5], node.transform.scale))
                return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Invalid scene node name or transform"});
            node.name = fields[2];
            if (!seen_source_ids.insert(node.source_id).second)
                return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Duplicate scene node id"});
            pending.push_back(std::move(node));
        } else {
            return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Unknown scene key '" + key + "'"});
        }
    }
    if (!version_seen || !has_version_one || !id_seen)
        return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene requires format_version=1 and scene_id"});
    if (pending.empty()) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene must contain at least one node"});

    // Validate all parent references and cycles before mutating the result graph.
    std::unordered_map<std::uint64_t, const PendingNode*> by_id;
    by_id.reserve(pending.size());
    for (const auto& node : pending) by_id.emplace(node.source_id, &node);
    for (const auto& node : pending) {
        if (node.source_parent && !by_id.contains(*node.source_parent))
            return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene contains a missing parent or hierarchy cycle"});
    }
    std::unordered_map<std::uint64_t, unsigned char> visit_state;
    visit_state.reserve(pending.size());
    for (const auto& node : pending) {
        if (visit_state[node.source_id] == 2) continue;
        std::vector<std::uint64_t> path;
        auto current = node.source_id;
        while (true) {
            const auto state = visit_state[current];
            if (state == 2) break;
            if (state == 1)
                return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene contains a missing parent or hierarchy cycle"});
            visit_state[current] = 1;
            path.push_back(current);
            const auto* current_node = by_id.at(current);
            if (!current_node->source_parent) break;
            current = *current_node->source_parent;
        }
        for (const auto visited : path) visit_state[visited] = 2;
    }

    std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> children_by_parent;
    children_by_parent.reserve(pending.size());
    for (const auto& node : pending) {
        if (node.source_parent) children_by_parent[*node.source_parent].push_back(node.source_id);
    }

    // Restore each root subtree using source IDs instead of allocating new IDs.
    // This keeps selection references, external entity links and editor history
    // stable across save/load, including scenes whose IDs contain gaps.
    SceneGraph graph;
    for (const auto& root : pending) {
        if (root.source_parent) continue;
        std::vector<SceneNode> snapshot;
        std::vector<std::uint64_t> stack{root.source_id};
        while (!stack.empty()) {
            const auto current = stack.back();
            stack.pop_back();
            const auto found = by_id.find(current);
            if (found == by_id.end()) continue;
            const auto& source = *found->second;
            SceneNode node;
            node.id = source.source_id;
            node.name = source.name;
            node.local_transform = source.transform;
            node.parent = source.source_parent;
            snapshot.push_back(std::move(node));
            const auto children = children_by_parent.find(current);
            if (children != children_by_parent.end()) {
                for (const auto child_id : children->second) stack.push_back(child_id);
            }
        }
        auto restored = graph.restore_subtree(snapshot);
        if (!restored) return Result<SceneGraph>::failure(restored.error());
    }
    if (graph.size() != pending.size())
        return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene contains a missing parent or hierarchy cycle"});
    return Result<SceneGraph>::success(std::move(graph));
}

} // namespace ecos::studio
