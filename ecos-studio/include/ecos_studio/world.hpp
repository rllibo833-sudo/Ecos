#pragma once

#include "ecos_studio/result.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace ecos::studio {

struct CellCoord {
    std::int64_t x{0};
    std::int64_t y{0};
    std::int64_t z{0};
    friend bool operator==(const CellCoord&, const CellCoord&) = default;
};

struct WorldCell {
    std::string id;
    std::string region_id;
    CellCoord coord{};
    std::filesystem::path content_path;
};

enum class CellState { unloaded, loading, loaded, failed };

struct CellStatus {
    WorldCell cell;
    CellState state{CellState::unloaded};
    std::string last_error;
};

// A deterministic, side-effect-free streaming decision. The caller performs
// unloads first, then requests loads, and completes IO asynchronously.
struct WorldStreamingPlan {
    std::vector<std::string> unload;
    std::vector<std::string> load;
};

class WorldRuntime {
public:
    explicit WorldRuntime(std::size_t max_loaded_cells = 64);
    [[nodiscard]] Result<void> register_cell(WorldCell cell);
    [[nodiscard]] Result<void> request_load(const std::string& id);
    [[nodiscard]] Result<void> complete_load(const std::string& id, bool success, std::string error = {});
    [[nodiscard]] Result<void> unload(const std::string& id);
    [[nodiscard]] const CellStatus* find(const std::string& id) const noexcept;
    [[nodiscard]] std::vector<std::string> cells_in_radius(CellCoord center, std::uint32_t radius) const;
    // Hysteresis is caller-controlled: unload_radius should normally be larger
    // than load_radius to avoid cells thrashing at a boundary.
    [[nodiscard]] WorldStreamingPlan plan_streaming(
        CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius) const;
    [[nodiscard]] std::size_t loaded_count() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] std::size_t max_loaded_cells() const noexcept;
    void set_max_loaded_cells(std::size_t limit) noexcept;

private:
    std::unordered_map<std::string, CellStatus> cells_;
    std::size_t max_loaded_cells_;
};

[[nodiscard]] const char* cell_state_name(CellState state) noexcept;

} // namespace ecos::studio
