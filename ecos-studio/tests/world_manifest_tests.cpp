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
}
int main() {
    test_valid_manifest();
    test_invalid_manifest();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio world manifest tests passed.\n";
    return 0;
}
