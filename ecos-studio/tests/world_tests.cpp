#include "ecos_studio/world.hpp"

#include <iostream>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_cell_lifecycle_and_budget() {
    using namespace ecos::studio;
    WorldRuntime world(1);
    check(world.register_cell({"arunika-0-0", "arunika", {0, 0, 0}, "world/arunika/cell-0-0.ecell"}),
          "registers a world cell");
    check(world.register_cell({"arunika-1-0", "arunika", {1, 0, 0}, "world/arunika/cell-1-0.ecell"}),
          "registers adjacent cell");
    check(!world.register_cell({"arunika-1-0", "arunika", {2, 0, 0}, "world/duplicate.ecell"}),
          "rejects duplicate cell id");
    check(!world.register_cell({"escape", "arunika", {0, 0, 0}, "../../outside.ecell"}),
          "rejects cell paths outside project");
    check(world.request_load("arunika-0-0"), "requests cell load");
    check(world.find("arunika-0-0")->state == CellState::loading, "moves cell into loading state");
    check(!world.request_load("arunika-1-0"), "enforces loading budget while first cell loads");
    check(world.complete_load("arunika-0-0", true), "marks cell loaded");
    check(world.loaded_count() == 1, "counts resident cells");
    check(world.unload("arunika-0-0"), "unloads resident cell");
    check(world.request_load("arunika-1-0"), "loads second cell after budget is freed");
    check(world.complete_load("arunika-1-0", false, "bad asset"), "records failed load result");
    check(world.find("arunika-1-0")->state == CellState::failed, "failed load has failed state");
    check(world.find("arunika-1-0")->last_error == "bad asset", "keeps diagnostic text");
    check(world.request_load("arunika-1-0"), "failed cell can be retried");
    check(world.complete_load("arunika-1-0", true), "retry can succeed");
}
void test_spatial_query() {
    using namespace ecos::studio;
    WorldRuntime world;
    check(world.register_cell({"c0", "r", {0, 0, 0}, "world/c0.ecell"}), "registers origin cell");
    check(world.register_cell({"c1", "r", {3, 0, 4}, "world/c1.ecell"}), "registers cell on radius edge");
    check(world.register_cell({"c2", "r", {6, 0, 0}, "world/c2.ecell"}), "registers distant cell");
    const auto nearby = world.cells_in_radius({0, 0, 0}, 5);
    check(nearby.size() == 2, "spatial query uses radius distance");
    check(nearby[0] == "c0" && nearby[1] == "c1", "spatial query results are deterministic");
}
void test_streaming_plan_prioritizes_nearest_and_frees_budget() {
    using namespace ecos::studio;
    WorldRuntime world(2);
    check(world.register_cell({"resident-far", "r", {20, 0, 0}, "world/far.ecell"}), "register far resident");
    check(world.register_cell({"near-b", "r", {2, 0, 0}, "world/near-b.ecell"}), "register near candidate B");
    check(world.register_cell({"near-a", "r", {1, 0, 0}, "world/near-a.ecell"}), "register near candidate A");
    check(world.register_cell({"outside", "r", {50, 0, 0}, "world/outside.ecell"}), "register outside candidate");
    check(world.request_load("resident-far"), "begin far resident load");
    check(world.complete_load("resident-far", true), "complete far resident load");

    const auto plan = world.plan_streaming({0, 0, 0}, 3, 5);
    check(plan.unload.size() == 1 && plan.unload.front() == "resident-far",
          "plans to unload resident outside hysteresis radius");
    check(plan.load.size() == 2 && plan.load[0] == "near-a" && plan.load[1] == "near-b",
          "prioritizes nearest cells and respects capacity released by unload");
    check(world.find("resident-far")->state == CellState::loaded,
          "planning is side-effect-free and leaves execution to runtime owner");
    check(world.find("near-a")->state == CellState::unloaded,
          "planning does not start IO implicitly");
}
void test_streaming_keeps_loading_cells_and_is_deterministic() {
    using namespace ecos::studio;
    WorldRuntime world(2);
    check(world.register_cell({"z", "r", {1, 0, 0}, "world/z.ecell"}), "register z");
    check(world.register_cell({"a", "r", {-1, 0, 0}, "world/a.ecell"}), "register a");
    check(world.register_cell({"in-flight", "r", {100, 0, 0}, "world/in-flight.ecell"}), "register in-flight");
    check(world.request_load("in-flight"), "start in-flight cell");
    const auto plan = world.plan_streaming({0, 0, 0}, 2, 3);
    check(plan.load.size() == 1 && plan.load[0] == "a",
          "does not exceed budget and breaks equal-distance ties by id");
    check(plan.unload.empty(), "never schedules in-flight loads for unload");
}
}
int main() {
    test_cell_lifecycle_and_budget();
    test_spatial_query();
    test_streaming_plan_prioritizes_nearest_and_frees_budget();
    test_streaming_keeps_loading_cells_and_is_deterministic();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio world runtime tests passed.\n";
    return 0;
}
