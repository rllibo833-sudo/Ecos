#include "ecos_studio/world_content.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace {
int failures = 0;
template <typename T> void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void write_file(const std::filesystem::path& path, const std::string& contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary); out << contents;
}
void test_loads_scene_payload() {
    using namespace ecos::studio;
    const auto root = std::filesystem::temp_directory_path() / "ecos-world-content-valid";
    std::filesystem::remove_all(root);
    write_file(root / "world.ecworld", "format_version=1\nworld_id=content-test\ncell=origin|region|0,0,0|cells/origin.ecell\n");
    write_file(root / "cells/origin.ecell", "format_version=1\nscene_id=origin\nnode=1|-|Spawn|0,0,0|0,0,0|1,1,1\nnode=2|1|Camera|0,1,4|0,0,0|1,1,1\n");
    auto manifest = load_world_manifest(root / "world.ecworld");
    check(manifest, "loads test world manifest");
    if (manifest) {
        auto loaded = load_world_cell_content(manifest.value(), manifest.value().cells.front());
        check(loaded, "loads and parses declared cell content");
        if (loaded) { check(loaded.value().cell_id == "origin", "preserves cell ID"); check(loaded.value().scene.size() == 2, "materializes scene nodes from cell file"); }
        check(!load_world_cell_content(manifest.value(), manifest.value().cells.front(), 8), "rejects content above configured byte limit");
    }
    std::filesystem::remove_all(root);
}
void test_rejects_missing_malformed_and_undeclared_content() {
    using namespace ecos::studio;
    const auto root = std::filesystem::temp_directory_path() / "ecos-world-content-invalid";
    std::filesystem::remove_all(root);
    write_file(root / "world.ecworld", "format_version=1\nworld_id=content-test\ncell=origin|region|0,0,0|cells/origin.ecell\n");
    auto manifest = load_world_manifest(root / "world.ecworld");
    check(manifest, "loads invalid-content fixture manifest");
    if (manifest) {
        check(!load_world_cell_content(manifest.value(), manifest.value().cells.front()), "reports missing cell content");
        write_file(root / "cells/origin.ecell", "format_version=77\nscene_id=broken\n");
        check(!load_world_cell_content(manifest.value(), manifest.value().cells.front()), "rejects malformed or unsupported scene content");
        WorldCell outsider{"outsider", "region", {1, 0, 0}, "cells/origin.ecell"};
        check(!load_world_cell_content(manifest.value(), outsider), "rejects a cell not declared by the manifest");
    }
    std::filesystem::remove_all(root);
}
void test_rejects_symlink_escape() {
    using namespace ecos::studio;
    const auto root = std::filesystem::temp_directory_path() / "ecos-world-content-escape";
    const auto outside = std::filesystem::temp_directory_path() / "ecos-world-content-outside.ecell";
    std::filesystem::remove_all(root); std::error_code ec; std::filesystem::remove(outside, ec);
    write_file(root / "world.ecworld", "format_version=1\nworld_id=content-test\ncell=origin|region|0,0,0|cells/linked.ecell\n");
    write_file(outside, "format_version=1\nscene_id=outside\nnode=1|-|Outside|0,0,0|0,0,0|1,1,1\n");
    std::filesystem::create_directories(root / "cells"); std::filesystem::create_symlink(outside, root / "cells/linked.ecell", ec);
    auto manifest = load_world_manifest(root / "world.ecworld");
    if (!ec && manifest) check(!load_world_cell_content(manifest.value(), manifest.value().cells.front()), "rejects symlink pointing outside the world root");
    std::filesystem::remove_all(root); std::filesystem::remove(outside, ec);
}
}
int main() {
    test_loads_scene_payload(); test_rejects_missing_malformed_and_undeclared_content(); test_rejects_symlink_escape();
    if (failures != 0) { std::cerr << "world content test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio world cell content tests passed.\n"; return 0;
}
