#include "ecos_studio/project.hpp"
#include "ecos_studio/world_manifest.hpp"
#include <filesystem>
#include <iostream>
#include <string>
namespace {
void print_usage() {
    std::cout << "ECOS Studio bootstrap 0.6.0\nUsage:\n"
              << "  ecos-studio --help\n"
              << "  ecos-studio new <directory> <project-name>\n"
              << "  ecos-studio validate <project-directory>\n"
              << "  ecos-studio mod-init <directory> <mod-id>\n"
              << "  ecos-studio mod-validate <mod-directory>\n"
              << "  ecos-studio world-validate <world.ecworld>\n\n"
              << "World validation checks schema version, cell IDs, coordinates, and safe content paths.\n";
}
int print_result(const ecos::studio::ValidationResult& result) {
    for (const auto& warning : result.warnings) std::cout << "info: " << warning << '\n';
    for (const auto& error : result.errors) std::cerr << "error: " << error << '\n';
    if (result.ok) { std::cout << "Valid.\n"; return 0; }
    return 2;
}
}
int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        print_usage(); return argc < 2 ? 1 : 0;
    }
    const std::string command = argv[1];
    std::string error;
    if (command == "new") {
        if (argc != 4) { print_usage(); return 1; }
        if (!ecos::studio::create_project(std::filesystem::path(argv[2]), argv[3], error)) {
            std::cerr << "error: " << error << '\n'; return 2;
        }
        std::cout << "Created ECOS Studio project at " << argv[2] << '\n'; return 0;
    }
    if (command == "validate") {
        if (argc != 3) { print_usage(); return 1; }
        return print_result(ecos::studio::validate_project(std::filesystem::path(argv[2])));
    }
    if (command == "mod-init") {
        if (argc != 4) { print_usage(); return 1; }
        if (!ecos::studio::create_mod(std::filesystem::path(argv[2]), argv[3], error)) {
            std::cerr << "error: " << error << '\n'; return 2;
        }
        std::cout << "Created data-only mod scaffold at " << argv[2] << '\n'; return 0;
    }
    if (command == "mod-validate") {
        if (argc != 3) { print_usage(); return 1; }
        return print_result(ecos::studio::validate_mod(std::filesystem::path(argv[2])));
    }
    if (command == "world-validate") {
        if (argc != 3) { print_usage(); return 1; }
        return print_result(ecos::studio::validate_world_manifest(std::filesystem::path(argv[2])));
    }
    std::cerr << "error: unknown command '" << command << "'\n";
    print_usage(); return 1;
}
