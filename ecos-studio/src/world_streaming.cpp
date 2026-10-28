#include "ecos_studio/world_streaming.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <utility>

namespace ecos::studio {

WorldStreamingService::WorldStreamingService(
    WorldRuntime& world, JobSystem& jobs, Loader loader, Unloader unloader)
    : world_(world), jobs_(jobs), loader_(std::move(loader)), unloader_(std::move(unloader)) {}

WorldStreamingService::~WorldStreamingService() {
    // Futures from JobSystem do not cancel queued work. Drain them explicitly so
    // the world cannot retain cells in the loading state after this service dies.
    for (auto& pending : pending_) {
        bool success = false;
        std::string error;
        pending.future.wait();
        try {
            auto result = pending.future.get();
            success = static_cast<bool>(result);
            if (!success) error = result.error().message;
        } catch (const std::exception& exception) {
            error = exception.what();
        } catch (...) {
            error = "World cell loader threw a non-standard exception";
        }
        (void)world_.complete_load(pending.id, success, std::move(error));
    }
}

Result<void> WorldStreamingService::schedule_load(const WorldCell& cell) {
    auto requested = world_.request_load(cell.id);
    if (!requested) return requested;
    try {
        auto future = jobs_.submit([loader = loader_, cell] { return loader(cell); });
        pending_.push_back(PendingLoad{cell.id, std::move(future)});
    } catch (const std::exception& exception) {
        auto failed = world_.complete_load(cell.id, false, exception.what());
        if (!failed) return failed;
        return Result<void>::failure({ErrorCode::internal_error, "Could not schedule world cell load: " + std::string(exception.what())});
    }
    return Result<void>::success();
}

Result<void> WorldStreamingService::retry_failed_cell(const std::string& id) {
    if (!loader_) {
        return Result<void>::failure({ErrorCode::invalid_state, "World streaming requires a cell loader callback"});
    }
    const auto* status = world_.find(id);
    if (!status) return Result<void>::failure({ErrorCode::not_found, "World cell does not exist: " + id});
    if (status->state != CellState::failed) {
        return Result<void>::failure({ErrorCode::invalid_state, "Only failed world cells can be retried"});
    }
    if (world_.loaded_count() >= world_.max_loaded_cells()) {
        return Result<void>::failure({ErrorCode::invalid_state, "World cell budget is full; unload a cell before retrying"});
    }
    return schedule_load(status->cell);
}

Result<StreamingUpdateReport> WorldStreamingService::collect_completed(StreamingUpdateReport report) {
    for (std::size_t i = 0; i < pending_.size();) {
        auto& pending = pending_[i];
        if (pending.future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            ++i;
            continue;
        }

        bool success = false;
        std::string error;
        try {
            auto result = pending.future.get();
            success = static_cast<bool>(result);
            if (!success) error = result.error().message;
        } catch (const std::exception& exception) {
            error = exception.what();
        } catch (...) {
            error = "World cell loader threw a non-standard exception";
        }

        auto completed = world_.complete_load(pending.id, success, std::move(error));
        if (!completed) return Result<StreamingUpdateReport>::failure(completed.error());
        if (success) ++report.completed_loads;
        else ++report.failed_loads;
        pending_.erase(pending_.begin() + static_cast<std::ptrdiff_t>(i));
    }
    return Result<StreamingUpdateReport>::success(report);
}

Result<StreamingUpdateReport> WorldStreamingService::update(
    CellCoord center, std::uint32_t load_radius, std::uint32_t unload_radius) {
    if (!loader_) {
        return Result<StreamingUpdateReport>::failure(
            {ErrorCode::invalid_state, "World streaming requires a cell loader callback"});
    }
    if (unload_radius < load_radius) {
        return Result<StreamingUpdateReport>::failure(
            {ErrorCode::invalid_argument, "Unload radius must be greater than or equal to load radius"});
    }

    auto collected = collect_completed({});
    if (!collected) return collected;
    auto report = collected.value();

    // Apply evictions before requesting new loads so freed capacity is reusable.
    const auto plan = world_.plan_streaming(center, load_radius, unload_radius);
    for (const auto& id : plan.unload) {
        const auto* status = world_.find(id);
        if (!status) continue;
        if (unloader_) {
            Result<void> released = Result<void>::success();
            try {
                released = unloader_(status->cell);
            } catch (const std::exception& exception) {
                return Result<StreamingUpdateReport>::failure(
                    {ErrorCode::internal_error, "World cell unloader threw: " + std::string(exception.what())});
            } catch (...) {
                return Result<StreamingUpdateReport>::failure(
                    {ErrorCode::internal_error, "World cell unloader threw a non-standard exception"});
            }
            if (!released) return Result<StreamingUpdateReport>::failure(released.error());
        }
        auto unloaded = world_.unload(id);
        if (!unloaded) return Result<StreamingUpdateReport>::failure(unloaded.error());
        ++report.unloaded_cells;
    }

    // Re-plan after eviction, then start bounded asynchronous loads.
    const auto load_plan = world_.plan_streaming(center, load_radius, unload_radius);
    for (const auto& id : load_plan.load) {
        const auto* status = world_.find(id);
        if (!status) continue;
        auto scheduled = schedule_load(status->cell);
        if (scheduled) ++report.started_loads;
        else if (scheduled.error().code == ErrorCode::internal_error) ++report.failed_loads;
        // A stale plan must not break the frame; invalid-state errors mean a
        // concurrent owner-side transition already made this candidate ineligible.
    }
    return Result<StreamingUpdateReport>::success(report);
}

std::size_t WorldStreamingService::in_flight_count() const noexcept { return pending_.size(); }
bool WorldStreamingService::idle() const noexcept { return pending_.empty(); }

} // namespace ecos::studio
