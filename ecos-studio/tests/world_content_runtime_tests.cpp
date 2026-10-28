#include "ecos_studio/world_content_runtime.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
int failures = 0;
template <typename T> void check(const T& value, const char* message) {
    if (!static_cast<bool>(value)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void write_file(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << content;
}
std::filesystem::path make_world(const std::string& name, bool broken = false) {
    const auto root = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(root);
    write_file(root / "world.ecworld",
        "format_version=1\nworld_id=runtime-test\n"
        "cell=origin|region|0,0,0|cells/origin.ecell\n"
        "cell=east|region|10,0,0|cells/east.ecell\n");
    write_file(root / "cells/origin.ecell",
        "format_version=1\nscene_id=origin\n"
        "node=1|-|Spawn|0,0,0|0,0,0|1,1,1\n"
        "node=2|1|Gate|2,0,0|0,0,0|1,1,1\n");
    write_file(root / "cells/east.ecell", broken
        ? "format_version=77\nscene_id=broken\n"
        : "format_version=1\nscene_id=east\nnode=1|-|East Landmark|0,0,0|0,0,0|1,1,1\n");
    return root;
}
void test_load_resident_snapshot_unload_and_reload() {
    using namespace ecos::studio;
    const auto root = make_world("ecos-content-runtime-lifecycle");
    auto manifest = load_world_manifest(root / "world.ecworld");
    check(manifest, "loads runtime fixture manifest");
    if (manifest) {
        JobSystem jobs(2);
        auto created = WorldContentRuntime::create(std::move(manifest.value()), jobs, 2);
        check(created, "creates integrated world content runtime");
        if (created) {
            auto runtime = std::move(created.value());
            auto started = runtime->update({0, 0, 0}, 0, 1);
            check(started && started.value().started_loads == 1, "schedules cell at the player position");
            jobs.wait_idle();
            auto completed = runtime->update({0, 0, 0}, 0, 1);
            check(completed && completed.value().completed_loads == 1, "completes content load");
            check(runtime->world().find("origin")->state == CellState::loaded, "marks origin cell loaded");
            check(runtime->resident_content_count() == 1, "stores parsed scene while cell is resident");
            auto scene = runtime->scene_snapshot("origin");
            check(scene && scene.value().size() == 2, "exposes a safe snapshot of the loaded scene");
            check(!runtime->scene_snapshot("east"), "does not expose an unloaded scene");
            auto move = runtime->update({10, 0, 0}, 0, 1);
            check(move && move.value().unloaded_cells == 1 && move.value().started_loads == 1,
                  "unloads departed cell and starts destination cell");
            check(runtime->resident_content_count() == 0 || runtime->resident_content_count() == 1,
                  "resident store changes only as async loading completes");
            jobs.wait_idle();
            auto arrived = runtime->update({10, 0, 0}, 0, 1);
            check(arrived && arrived.value().completed_loads == 1, "completes destination content load");
            check(!runtime->scene_snapshot("origin"), "evicted scene is no longer accessible");
            check(runtime->scene_snapshot("east"), "destination scene becomes accessible");
            auto return_trip = runtime->update({0, 0, 0}, 0, 1);
            check(return_trip && return_trip.value().unloaded_cells == 1 && return_trip.value().started_loads == 1,
                  "supports unloading and reloading a previous cell");
            jobs.wait_idle();
            auto returned = runtime->update({0, 0, 0}, 0, 1);
            check(returned && returned.value().completed_loads == 1, "reloaded cell completes");
            check(runtime->scene_snapshot("origin"), "reloaded scene can be queried again");
        }
    }
    std::filesystem::remove_all(root);
}
void test_bad_content_is_failed_and_can_retry() {
    using namespace ecos::studio;
    const auto root = make_world("ecos-content-runtime-retry", true);
    auto manifest = load_world_manifest(root / "world.ecworld");
    check(manifest, "loads retry fixture manifest");
    if (manifest) {
        JobSystem jobs(1);
        auto created = WorldContentRuntime::create(std::move(manifest.value()), jobs, 1);
        check(created, "creates retry runtime");
        if (created) {
            auto runtime = std::move(created.value());
            auto start = runtime->update({10, 0, 0}, 0, 0);
            check(start && start.value().started_loads == 1, "starts invalid content load");
            jobs.wait_idle();
            auto finish = runtime->update({10, 0, 0}, 0, 0);
            check(finish && finish.value().failed_loads == 1, "surfaces parse failure through streaming");
            check(runtime->world().find("east")->state == CellState::failed, "retains failed cell state");
            check(runtime->resident_content_count() == 0, "does not retain invalid content");
            write_file(root / "cells/east.ecell",
                "format_version=1\nscene_id=east\nnode=1|-|Recovered|0,0,0|0,0,0|1,1,1\n");
            check(runtime->retry_failed_cell("east"), "explicit retry is available after content is repaired");
            jobs.wait_idle();
            auto retried = runtime->update({10, 0, 0}, 0, 0);
            check(retried && retried.value().completed_loads == 1, "retry recovers and completes load");
            check(runtime->scene_snapshot("east"), "recovered scene is queryable");
        }
    }
    std::filesystem::remove_all(root);
}
}
int main() {
    test_load_resident_snapshot_unload_and_reload();
    test_bad_content_is_failed_and_can_retry();
    if (failures != 0) { std::cerr << failures << " world content runtime test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio world content runtime tests passed.\n";
    return 0;
}
