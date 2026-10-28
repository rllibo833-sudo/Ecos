#include "ecos_studio/world.hpp"
#include "ecos_studio/platform.hpp"

#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>

namespace ecos::studio {
namespace {
long double distance_squared(CellCoord a, CellCoord b) noexcept {
    // Convert before subtraction so signed 64-bit coordinate differences
    // cannot overflow before being widened.
    const long double dx = static_cast<long double>(a.x) - static_cast<long double>(b.x);
    const long double dy = static_cast<long double>(a.y) - static_cast<long double>(b.y);
    const long double dz = static_cast<long double>(a.z) - static_cast<long double>(b.z);
    return dx * dx + dy * dy + dz * dz;
}
bool inside_radius(CellCoord point, CellCoord center, std::uint32_t radius) noexcept {
    const long double r = static_cast<long double>(radius);
    return distance_squared(point, center) <= r * r;
}
}

WorldRuntime::WorldRuntime(std::size_t max_loaded_cells)
    : max_loaded_cells_(std::max<std::size_t>(1, max_loaded_cells)) {}

Result<void> WorldRuntime::register_cell(WorldCell cell) {
    if (cell.id.empty() || cell.region_id.empty()) {
        return Result<void>::failure({ErrorCode::invalid_argument, "World cell id and region id must not be empty"});
    }
    if (cells_.find(cell.id) != cells_.end()) {
        return Result<void>::failure({ErrorCode::invalid_argument, "Duplicate world cell id: " + cell.id});
    }
    auto safe_path = platform::safe_relative_path(cell.content_path);
    if (!safe_path) return Result<void>::failure(safe_path.error());
    cell.content_path = safe_path.value();
    const std::string id = cell.id;
    cells_.emplace(id, CellStatus{std::move(cell), CellState::unloaded, {}});
    return Result<void>::success();
}

Result<void> WorldRuntime::request_load(const std::string& id) {
    auto it = cells_.find(id);
    if (it == cells_.end()) return Result<void>::failure({ErrorCode::not_found, "World cell does not exist: " + id});
    if (it->second.state != CellState::unloaded && it->second.state != CellState::failed) {
        return Result<void>::failure({ErrorCode::invalid_state, "World cell is not eligible to begin loading"});
    }
    if (loaded_count() >= max_loaded_cells_) {
        return Result<void>::failure({ErrorCode::invalid_state, "World cell budget is full; unload a cell before loading another"});
    }
    it->second.state = CellState::loading;
    it->second.last_error.clear();
    return Result<void>::success();
}

Result<void> WorldRuntime::complete_load(const std::string& id, bool success, std::string error) {
    auto it = cells_.find(id);
    if (it == cells_.end()) return Result<void>::failure({ErrorCode::not_found, "World cell does not exist: " + id});
    if (it->second.state != CellState::loading) {
        return Result<void>::failure({ErrorCode::invalid_state, "World cell completion requires loading state"});
    }
    it->second.state = success ? CellState::loaded : CellState::failed;
    it->second.last_error = success ? std::string{} :
        (error.empty() ? "Cell content loading failed" : std::move(error));
    return Result<void>::success();
}

Result<void> WorldRuntime::unload(const std::string& id) {
    auto it = cells_.find(id);
    if (it == cells_.end()) return Result<void>::failure({ErrorCode::not_found, "World cell does not exist: " + id});
    if (it->second.state == CellState::loading) {
        return Result<void>::failure({ErrorCode::invalid_state, "Cannot unload a cell while loading is in progress"});
    }
    if (it->second.state == CellState::unloaded) return Result<void>::success();
    it->second.state = CellState::unloaded;
    it->second.last_error.clear();
    return Result<void>::success();
}

const CellStatus* WorldRuntime::find(const std::string& id) const noexcept {
    const auto it = cells_.find(id);
    return it == cells_.end() ? nullptr : &it->second;
}

std::vector<std::string> WorldRuntime::cells_in_radius(CellCoord center, std::uint32_t radius) const {
    std::vector<std::string> result;
    for (const auto& [id, status] : cells_) {
        if (inside_radius(status.cell.coord, center, radius)) result.push_back(id);
    }
    std::sort(result.begin(), result.end());
    return result;
}

WorldStreamingPlan WorldRuntime::plan_streaming(
    CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius) const {
    WorldStreamingPlan plan;
    struct Candidate { std::string id; long double distance; };
    std::vector<Candidate> candidates;
    std::size_t resident = 0;

    for (const auto& [id, status] : cells_) {
        const auto state = status.state;
        if (state == CellState::loaded || state == CellState::loading) ++resident;
        const auto distance = distance_squared(status.cell.coord, center);
        if (state == CellState::loaded && !inside_radius(status.cell.coord, center, unload_radius)) {
            plan.unload.push_back(id);
        } else if ((state == CellState::unloaded || state == CellState::failed) &&
                   inside_radius(status.cell.coord, center, load_radius)) {
            candidates.push_back({id, distance});
        }
    }

    // Stable ordering makes streaming behavior reproducible across platforms.
    std::sort(plan.unload.begin(), plan.unload.end());
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        return a.id < b.id;
    });

    const std::size_t after_unloads = resident - plan.unload.size();
    std::size_t capacity = max_loaded_cells_ > after_unloads ? max_loaded_cells_ - after_unloads : 0;
    for (const auto& candidate : candidates) {
        if (capacity == 0) break;
        plan.load.push_back(candidate.id);
        --capacity;
    }
    return plan;
}

std::size_t WorldRuntime::loaded_count() const noexcept {
    std::size_t count = 0;
    for (const auto& [id, status] : cells_) {
        (void)id;
        if (status.state == CellState::loaded || status.state == CellState::loading) ++count;
    }
    return count;
}
std::size_t WorldRuntime::size() const noexcept { return cells_.size(); }
std::size_t WorldRuntime::max_loaded_cells() const noexcept { return max_loaded_cells_; }
void WorldRuntime::set_max_loaded_cells(std::size_t limit) noexcept {
    max_loaded_cells_ = std::max<std::size_t>(1, limit);
}

const char* cell_state_name(CellState state) noexcept {
    switch (state) {
    case CellState::unloaded: return "unloaded";
    case CellState::loading: return "loading";
    case CellState::loaded: return "loaded";
    case CellState::failed: return "failed";
    }
    return "unknown";
}

} // namespace ecos::studio
