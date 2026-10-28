#include "ecos_studio/world_streaming.hpp"

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_async_loads_and_main_thread_unloads() {
    using namespace ecos::studio;
    WorldRuntime world(2);
    check(world.register_cell({"near-a", "region", {0, 0, 0}, "world/a.ecell"}), "registers first cell");
    check(world.register_cell({"near-b", "region", {1, 0, 0}, "world/b.ecell"}), "registers second cell");
    check(world.register_cell({"far", "region", {20, 0, 0}, "world/far.ecell"}), "registers far cell");
    JobSystem jobs(2);
    std::atomic<int> loaded{0};
    std::atomic<int> unloaded{0};
    WorldStreamingService streaming(
        world, jobs,
        [&loaded](const WorldCell&) {
            loaded.fetch_add(1, std::memory_order_relaxed);
            return Result<void>::success();
        },
        [&unloaded](const WorldCell&) {
            unloaded.fetch_add(1, std::memory_order_relaxed);
            return Result<void>::success();
        });

    auto first = streaming.update({0, 0, 0}, 2, 4);
    check(first && first.value().started_loads == 2, "schedules nearest cells asynchronously");
    jobs.wait_idle();
    auto second = streaming.update({0, 0, 0}, 2, 4);
    check(second && second.value().completed_loads == 2, "collects completed worker results on update");
    check(world.find("near-a")->state == CellState::loaded &&
          world.find("near-b")->state == CellState::loaded, "marks successful cells resident");
    check(loaded.load(std::memory_order_relaxed) == 2, "runs loader exactly once per requested cell");

    auto third = streaming.update({20, 0, 0}, 1, 3);
    check(third && third.value().unloaded_cells == 2, "evicts cells outside unload radius");
    check(unloaded.load(std::memory_order_relaxed) == 2, "invokes unloader for each evicted cell");
    check(world.find("near-a")->state == CellState::unloaded, "evicted cell returns to unloaded state");
    check(third && third.value().started_loads == 1, "uses freed budget to load nearby destination");
    jobs.wait_idle();
    auto fourth = streaming.update({20, 0, 0}, 1, 3);
    check(fourth && fourth.value().completed_loads == 1, "completes destination load");
}
void test_failures_require_explicit_retry() {
    using namespace ecos::studio;
    WorldRuntime world(2);
    check(world.register_cell({"broken", "region", {0, 0, 0}, "world/broken.ecell"}), "registers broken cell");
    JobSystem jobs(1);
    std::atomic<int> attempts{0};
    WorldStreamingService streaming(world, jobs, [&attempts](const WorldCell&) -> Result<void> {
        attempts.fetch_add(1, std::memory_order_relaxed);
        return Result<void>::failure({ErrorCode::io_error, "simulated read failure"});
    });
    check(!streaming.update({0, 0, 0}, 5, 2), "rejects unload radius smaller than load radius");
    auto started = streaming.update({0, 0, 0}, 1, 2);
    check(started && started.value().started_loads == 1, "starts failed-load fixture");
    jobs.wait_idle();
    auto finished = streaming.update({0, 0, 0}, 1, 2);
    check(finished && finished.value().failed_loads == 1, "reports asynchronous load failure");
    check(world.find("broken")->state == CellState::failed, "retains failure state for retry");
    check(world.find("broken")->last_error == "simulated read failure", "preserves loader diagnostic");
    check(finished && finished.value().started_loads == 0, "does not spin on failed cell every frame");
    check(attempts.load(std::memory_order_relaxed) == 1, "failed cell is not retried automatically");
    check(streaming.retry_failed_cell("broken"), "allows an explicit retry");
    jobs.wait_idle();
    auto retried = streaming.update({0, 0, 0}, 1, 2);
    check(retried && retried.value().failed_loads == 1, "reports retry outcome");
    check(attempts.load(std::memory_order_relaxed) == 2, "explicit retry schedules one new attempt");
    check(!streaming.retry_failed_cell("unknown"), "rejects retry for unknown cell");
}
void test_loader_exceptions_are_captured() {
    using namespace ecos::studio;
    WorldRuntime world(1);
    check(world.register_cell({"throws", "region", {0, 0, 0}, "world/throws.ecell"}), "registers throwing cell");
    JobSystem jobs(1);
    WorldStreamingService streaming(world, jobs, [](const WorldCell&) -> Result<void> {
        throw std::runtime_error("loader exploded");
    });
    auto start = streaming.update({0, 0, 0}, 0, 0);
    check(start, "starts throwing loader");
    jobs.wait_idle();
    auto finish = streaming.update({0, 0, 0}, 0, 0);
    check(finish && finish.value().failed_loads == 1, "captures worker exception as failed cell");
    check(world.find("throws")->last_error == "loader exploded", "stores exception message");
    check(finish && finish.value().started_loads == 0, "does not repeat thrown loads every frame");
}

void test_service_shutdown_settles_in_flight_loads() {
    using namespace ecos::studio;
    WorldRuntime world(1);
    check(world.register_cell({"shutdown", "region", {0, 0, 0}, "world/shutdown.ecell"}),
          "registers shutdown cell");
    JobSystem jobs(1);
    std::atomic<int> attempts{0};
    {
        WorldStreamingService streaming(world, jobs, [&attempts](const WorldCell&) {
            attempts.fetch_add(1, std::memory_order_relaxed);
            return Result<void>::success();
        });
        auto started = streaming.update({0, 0, 0}, 0, 0);
        check(started && started.value().started_loads == 1, "starts load before service shutdown");
        check(world.find("shutdown")->state == CellState::loading, "cell is loading before service shutdown");
    }
    check(attempts.load(std::memory_order_relaxed) == 1, "shutdown drains the in-flight loader");
    check(world.find("shutdown")->state == CellState::loaded,
          "shutdown reconciles completed load instead of leaving an orphan loading state");
}
}
int main() {
    test_async_loads_and_main_thread_unloads();
    test_failures_require_explicit_retry();
    test_loader_exceptions_are_captured();
    test_service_shutdown_settles_in_flight_loads();
    if (failures != 0) { std::cerr << failures << " streaming test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio asynchronous world streaming tests passed.\n";
    return 0;
}
