#pragma once

#include "ecos_studio/job_system.hpp"
#include "ecos_studio/result.hpp"
#include "ecos_studio/world.hpp"

#include <cstddef>
#include <future>
#include <functional>
#include <string>
#include <vector>

namespace ecos::studio {

struct StreamingUpdateReport {
    std::size_t completed_loads{0};
    std::size_t failed_loads{0};
    std::size_t unloaded_cells{0};
    std::size_t started_loads{0};
};

// Bridges deterministic world policy to asynchronous content IO. WorldRuntime
// is mutated only by update() on the caller thread; loader callbacks run on
// workers, while unloader callbacks run on the caller thread (GPU/resource
// destruction generally belongs to the render/device thread).
class WorldStreamingService {
public:
    using Loader = std::function<Result<void>(const WorldCell&)>;
    using Unloader = std::function<Result<void>(const WorldCell&)>;

    WorldStreamingService(WorldRuntime& world, JobSystem& jobs, Loader loader, Unloader unloader = {});
    WorldStreamingService(const WorldStreamingService&) = delete;
    WorldStreamingService& operator=(const WorldStreamingService&) = delete;

    [[nodiscard]] Result<StreamingUpdateReport> update(
        CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius);
    [[nodiscard]] std::size_t in_flight_count() const noexcept;
    [[nodiscard]] bool idle() const noexcept;

private:
    struct PendingLoad {
        std::string id;
        std::future<Result<void>> future;
    };

    [[nodiscard]] Result<StreamingUpdateReport> collect_completed(StreamingUpdateReport report);

    WorldRuntime& world_;
    JobSystem& jobs_;
    Loader loader_;
    Unloader unloader_;
    std::vector<PendingLoad> pending_;
};

} // namespace ecos::studio
