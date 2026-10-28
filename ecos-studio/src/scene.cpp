#include "ecos_studio/scene.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <sstream>
#include <tuple>
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
            if (std::any_of(pending.begin(), pending.end(), [&](const PendingNode& old) { return old.source_id == node.source_id; }))
                return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Duplicate scene node id"});
            pending.push_back(std::move(node));
        } else {
            return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Unknown scene key '" + key + "'"});
        }
    }
    if (!version_seen || !has_version_one || !id_seen)
        return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene requires format_version=1 and scene_id"});
    if (pending.empty()) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene must contain at least one node"});

    SceneGraph graph;
    std::unordered_map<std::uint64_t, NodeId> id_map;
    std::vector<bool> created(pending.size(), false);
    std::size_t remaining = pending.size();
    while (remaining > 0) {
        bool progress = false;
        for (std::size_t i = 0; i < pending.size(); ++i) {
            if (created[i]) continue;
            const auto& node = pending[i];
            std::optional<NodeId> parent;
            if (node.source_parent) {
                const auto found = id_map.find(*node.source_parent);
                if (found == id_map.end()) continue;
                parent = found->second;
            }
            auto created_node = graph.create_node(node.name, parent);
            if (!created_node) return Result<SceneGraph>::failure(created_node.error());
            auto set_transform = graph.set_local_transform(created_node.value(), node.transform);
            if (!set_transform) return Result<SceneGraph>::failure(set_transform.error());
            id_map.emplace(node.source_id, created_node.value());
            created[i] = true;
            --remaining;
            progress = true;
        }
        if (!progress) return Result<SceneGraph>::failure({ErrorCode::invalid_argument, "Scene contains a missing parent or hierarchy cycle"});
    }
    return Result<SceneGraph>::success(std::move(graph));
}

} // namespace ecos::studio
