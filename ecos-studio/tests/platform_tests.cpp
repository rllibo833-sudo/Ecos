#include "ecos_studio/platform.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    using namespace ecos::studio;

    auto valid = platform::safe_relative_path("assets/meshes/tree.mesh");
    assert(valid);
    assert(valid.value() == std::filesystem::path("assets/meshes/tree.mesh"));

    auto traversal = platform::safe_relative_path("../../outside.txt");
    assert(!traversal);
    assert(traversal.error().code == ErrorCode::path_escape);

    auto absolute = platform::safe_relative_path(std::filesystem::current_path());
    assert(!absolute);
    assert(absolute.error().code == ErrorCode::path_escape);

    auto empty = platform::safe_relative_path("");
    assert(!empty);
    assert(empty.error().code == ErrorCode::invalid_argument);

    const auto before = platform::monotonic_time_ms();
    const auto after = platform::monotonic_time_ms();
    assert(after >= before);
    assert(!platform::operating_system_name().empty());

    const auto temp = std::filesystem::temp_directory_path() / "ecos-studio-platform-test";
    auto created = platform::ensure_directory(temp);
    assert(created);
    const auto file = temp / "sample.txt";
    {
        std::ofstream output(file, std::ios::binary);
        output << "ECOS Studio";
    }
    auto content = platform::read_text_file(file);
    assert(content);
    assert(content.value() == "ECOS Studio");

    auto missing = platform::read_text_file(temp / "missing.txt");
    assert(!missing);
    assert(missing.error().code == ErrorCode::io_error);

    std::error_code ignored;
    std::filesystem::remove_all(temp, ignored);
    return 0;
}
