#include "ecos_studio/world_manifest.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
std::filesystem::path write_temp(const std::string& name, const std::string& content) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream output(path, std::ios::binary);
    output << content;
    return path;
}
void test_valid_manifest() {
    using namespace ecos::studio;
    const auto path = write_temp("ecos-world-valid.ecworld",
        "format_version=1\nworld_id=demo\n"
        "cell=origin|region-a|0,0,0|world/origin.ecell\n"
        "cell=east|region-a|1,0,0|world/east.ecell\n");
    const auto result = validate_world_manifest(path);
    check(result.ok, "accepts valid world manifest");
    check(!result.warnings.empty(), "reports validated world summary");
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}
void test_invalid_manifest() {
    using namespace ecos::studio;
    const auto path = write_temp("ecos-world-invalid.ecworld",
        "format_version=9\nworld_id=demo\n"
        "cell=origin|region-a|0,0,0|world/origin.ecell\n"
        "cell=duplicate-coord|region-a|0,0,0|world/duplicate.ecell\n"
        "cell=escape|region-a|1,0,0|../../outside.ecell\n");
    const auto result = validate_world_manifest(path);
    check(!result.ok, "rejects invalid manifest");
    check(result.errors.size() >= 3, "reports schema, duplicate-coordinate, and unsafe-path errors");
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

void test_manifest_loads_runtime_cells() {
    using namespace ecos::studio;
    const auto path = write_temp("ecos-world-runtime.ecworld",
        "format_version=1\nworld_id=runtime-demo\n"
        "cell=origin|region-a|0,0,0|world/origin.ecell\n"
        "cell=east|region-a|1,0,0|world/east.ecell\n");
    auto manifest = load_world_manifest(path);
    check(manifest, "loads a typed world manifest");
    if (manifest) {
        check(manifest.value().world_id == "runtime-demo", "preserves world ID");
        check(manifest.value().cells.size() == 2, "parses all declared cells");
        auto runtime = manifest.value().create_runtime(4);
        check(runtime, "instantiates runtime from manifest");
        if (runtime) {
            check(runtime.value().size() == 2, "runtime registers every manifest cell");
            check(runtime.value().find("east") != nullptr, "runtime exposes manifest cell by ID");
            check(runtime.value().max_loaded_cells() == 4, "passes runtime cell budget");
        }
    }
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}
void test_loader_rejects_unsafe_or_duplicate_cells() {
    using namespace ecos::studio;
    const auto unsafe = write_temp("ecos-world-runtime-unsafe.ecworld",
        "format_version=1\nworld_id=unsafe\ncell=bad|region|0,0,0|../../escape.ecell\n");
    check(!load_world_manifest(unsafe), "typed loader rejects path traversal");
    const auto duplicate = write_temp("ecos-world-runtime-duplicate.ecworld",
        "format_version=1\nworld_id=duplicate\n"
        "cell=same|region|0,0,0|world/a.ecell\n"
        "cell=same|region|1,0,0|world/b.ecell\n");
    check(!load_world_manifest(duplicate), "typed loader rejects duplicate cell IDs");
    std::error_code ignored;
    std::filesystem::remove(unsafe, ignored);
    std::filesystem::remove(duplicate, ignored);
}
}
int main() {
    test_valid_manifest();
    test_invalid_manifest();
    test_manifest_loads_runtime_cells();
    test_loader_rejects_unsafe_or_duplicate_cells();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio world manifest tests passed.\n";
    return 0;
}
