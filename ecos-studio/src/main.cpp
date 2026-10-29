#include "ecos_studio/asset_importer.hpp"
#include "ecos_studio/camera.hpp"
#include "ecos_studio/job_system.hpp"
#include "ecos_studio/world_content_runtime.hpp"
#include "ecos_studio/mesh.hpp"
#include "ecos_studio/package_manager.hpp"
#include "ecos_studio/platform.hpp"
#include "ecos_studio/project.hpp"
#include "ecos_studio/render.hpp"
#include "ecos_studio/scene.hpp"
#include "ecos_studio/world_manifest.hpp"
#include "ecos_studio/world_content.hpp"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace {
void print_usage() {
    std::cout << "ECOS Studio 0.9.0\nUsage:\n"
              << "  ecos-studio --help\n"
              << "  ecos-studio new <directory> <project-name>\n"
              << "  ecos-studio validate <project-directory>\n"
              << "  ecos-studio mod-init <directory> <mod-id>\n"
              << "  ecos-studio mod-validate <mod-directory>\n"
              << "  ecos-studio mod-resolve <mods-root-directory>\n"
              << "  ecos-studio asset-import <project-directory> <source.obj> <asset-id>\n"
              << "  ecos-studio material-import <project-directory> <source.mtl> <asset-id>\n"
              << "  ecos-studio asset-validate <project-directory>\n"
              << "  ecos-studio scene-validate <scene.ecscene>\n"
              << "  ecos-studio world-validate <world.ecworld>\n"
              << "  ecos-studio world-inspect <world.ecworld>\n"
              << "  ecos-studio world-validate-content <world.ecworld>\n"
              << "  ecos-studio mesh-render <mesh.obj> <output.ppm> [width height]\n"
              << "  ecos-studio scene-render <project-directory> <scene.ecscene> <output.ppm> [width height]\n"
              << "  ecos-studio world-render <world.ecworld> <project-directory> <output.ppm> [width height]\n";
}
int print_result(const ecos::studio::ValidationResult& result) {
    for (const auto& warning : result.warnings) std::cout << "info: " << warning << '\n';
    for (const auto& error : result.errors) std::cerr << "error: " << error << '\n';
    if (result.ok) { std::cout << "Valid.\n"; return 0; }
    return 2;
}
bool parse_dimension(const char* text, std::uint32_t& value) {
    const std::string input(text);
    std::uint32_t parsed{};
    const auto result = std::from_chars(input.data(), input.data()+input.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != input.data()+input.size()) return false;
    value=parsed;
    return true;
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
    if (command == "mod-resolve") {
        if (argc != 3) { print_usage(); return 1; }
        const auto resolved = ecos::studio::resolve_mod_dependencies(std::filesystem::path(argv[2]));
        for (const auto& warning : resolved.warnings) std::cout << "warning: " << warning << '\n';
        for (const auto& message : resolved.errors) std::cerr << "error: " << message << '\n';
        if (!resolved.ok) return 2;
        std::cout << "Resolved " << resolved.load_order.size() << " mod package(s), dependency-first load order:\n";
        for (std::size_t index = 0; index < resolved.load_order.size(); ++index)
            std::cout << "  " << (index + 1) << ". " << resolved.load_order[index] << '\n';
        return 0;
    }
    if (command == "asset-import") {
        if (argc != 5) { print_usage(); return 1; }
        auto imported = ecos::studio::import_obj_asset(std::filesystem::path(argv[2]),
                                                       std::filesystem::path(argv[3]), argv[4]);
        if (!imported) { std::cerr << "error: " << imported.error().message << '\n'; return 2; }
        const auto& asset = imported.value();
        std::cout << "Imported mesh '" << asset.id << "' to " << asset.relative_path.generic_string()
                  << " (" << asset.vertex_count << " vertices, " << asset.triangle_count
                  << " triangles, SHA-256 " << asset.content_sha256 << ").\n";
        return 0;
    }
    if (command == "material-import") {
        if (argc != 5) { print_usage(); return 1; }
        auto imported = ecos::studio::import_mtl_asset(std::filesystem::path(argv[2]),
                                                       std::filesystem::path(argv[3]), argv[4]);
        if (!imported) { std::cerr << "error: " << imported.error().message << '\\n'; return 2; }
        const auto& asset = imported.value();
        std::cout << "Imported material library '" << asset.id << "' to " << asset.relative_path.generic_string()
                  << " (" << asset.material_count << " material(s), SHA-256 " << asset.content_sha256 << ").\\n";
        return 0;
    }
    if (command == "asset-validate") {
        if (argc != 3) { print_usage(); return 1; }
        const auto result = ecos::studio::validate_imported_assets(std::filesystem::path(argv[2]));
        if (!result.ok) {
            for (const auto& message : result.errors) std::cerr << "error: " << message << '\n';
            return 2;
        }
        std::cout << "Imported asset metadata, source hashes, and mesh statistics are valid.\n";
        return 0;
    }
    if (command == "scene-validate") {
        if (argc != 3) { print_usage(); return 1; }
        auto contents = ecos::studio::platform::read_text_file(std::filesystem::path(argv[2]));
        if (!contents) { std::cerr << "error: " << contents.error().message << '\n'; return 2; }
        auto scene = ecos::studio::deserialize_scene(contents.value());
        if (!scene) { std::cerr << "error: " << scene.error().message << '\n'; return 2; }
        std::cout << "Valid scene with " << scene.value().size() << " node(s).\n";
        return 0;
    }
    if (command == "world-validate") {
        if (argc != 3) { print_usage(); return 1; }
        return print_result(ecos::studio::validate_world_manifest(std::filesystem::path(argv[2])));
    }
    if (command == "world-inspect") {
        if (argc != 3) { print_usage(); return 1; }
        auto manifest = ecos::studio::load_world_manifest(std::filesystem::path(argv[2]));
        if (!manifest) { std::cerr << "error: " << manifest.error().message << '\n'; return 2; }
        auto runtime = manifest.value().create_runtime();
        if (!runtime) { std::cerr << "error: " << runtime.error().message << '\n'; return 2; }
        std::cout << "World '" << manifest.value().world_id << "' (" << runtime.value().size()
                  << " cells, budget " << runtime.value().max_loaded_cells() << ")\n";
        for (const auto& cell : manifest.value().cells) {
            std::cout << "  " << cell.id << " [" << cell.region_id << "] @ "
                      << cell.coord.x << ',' << cell.coord.y << ',' << cell.coord.z
                      << " -> " << cell.content_path.generic_string() << '\n';
        }
        return 0;
    }
    if (command == "world-validate-content") {
        if (argc != 3) { print_usage(); return 1; }
        auto manifest = ecos::studio::load_world_manifest(std::filesystem::path(argv[2]));
        if (!manifest) { std::cerr << "error: " << manifest.error().message << '\n'; return 2; }
        std::size_t nodes = 0;
        for (const auto& cell : manifest.value().cells) {
            auto content = ecos::studio::load_world_cell_content(manifest.value(), cell);
            if (!content) { std::cerr << "error: " << content.error().message << '\n'; return 2; }
            nodes += content.value().scene.size();
            std::cout << "  " << cell.id << ": " << content.value().scene.size() << " scene node(s)\n";
        }
        std::cout << "Validated " << manifest.value().cells.size() << " cell(s), " << nodes << " total scene node(s).\n";
        return 0;
    }
    if (command == "world-render") {
        if (argc != 5 && argc != 7) { print_usage(); return 1; }
        std::uint32_t width = 640, height = 480;
        if (argc == 7 && (!parse_dimension(argv[5], width) ||
            !parse_dimension(argv[6], height) || width == 0 || height == 0 ||
            width > 4096 || height > 4096)) {
            std::cerr << "error: width and height must be integers between 1 and 4096\\n";
            return 2;
        }
        auto manifest = ecos::studio::load_world_manifest(std::filesystem::path(argv[2]));
        if (!manifest) { std::cerr << "error: " << manifest.error().message << '\\n'; return 2; }
        if (manifest.value().cells.empty()) {
            std::cerr << "error: world manifest contains no cells\\n";
            return 2;
        }
        const auto center = manifest.value().cells.front().coord;
        ecos::studio::JobSystem jobs(2);
        auto created = ecos::studio::WorldContentRuntime::create(
            std::move(manifest.value()), jobs);
        if (!created) { std::cerr << "error: " << created.error().message << '\\n'; return 2; }
        auto runtime = std::move(created.value());
        auto scheduled = runtime->update(center, 1, 1);
        if (!scheduled) { std::cerr << "error: " << scheduled.error().message << '\\n'; return 2; }
        jobs.wait_idle();
        auto completed = runtime->update(center, 1, 1);
        if (!completed) { std::cerr << "error: " << completed.error().message << '\\n'; return 2; }
        if (runtime->world().loaded_count() == 0) {
            std::cerr << "error: first world cell failed to load";
            const auto* status = runtime->world().find(runtime->manifest().cells.front().id);
            if (status && !status->last_error.empty()) std::cerr << ": " << status->last_error;
            std::cerr << '\\n';
            return 2;
        }
        auto assets = ecos::studio::load_imported_asset_registry(std::filesystem::path(argv[3]));
        if (!assets) { std::cerr << "error: " << assets.error().message << '\\n'; return 2; }
        const ecos::studio::Vec3 target{
            static_cast<float>(center.x), static_cast<float>(center.y), static_cast<float>(center.z)};
        const ecos::studio::Vec3 eye{target.x + 3.0F, target.y + 3.0F, target.z + 5.0F};
        auto view = ecos::studio::look_at(eye, target, {0.0F, 1.0F, 0.0F});
        if (!view) { std::cerr << "error: " << view.error().message << '\\n'; return 2; }
        auto projection = ecos::studio::perspective(
            1.0471975512F, static_cast<float>(width) / static_cast<float>(height), 0.1F, 1000.0F);
        if (!projection) { std::cerr << "error: " << projection.error().message << '\\n'; return 2; }
        const auto view_projection = ecos::studio::multiply(projection.value(), view.value());
        auto image = ecos::studio::render_world_content_runtime_to_ppm(
            *runtime, assets.value(), std::filesystem::path(argv[3]),
            view_projection, width, height);
        if (!image) { std::cerr << "error: " << image.error().message << '\\n'; return 2; }
        std::ofstream output(std::filesystem::path(argv[4]), std::ios::binary);
        if (!output) { std::cerr << "error: cannot open output image for writing\\n"; return 2; }
        output.write(image.value().data(), static_cast<std::streamsize>(image.value().size()));
        if (!output) { std::cerr << "error: failed while writing output image\\n"; return 2; }
        std::cout << "Rendered streamed world cell '" << runtime->manifest().cells.front().id
                  << "' with " << runtime->entity_count() << " runtime entity(s) to "
                  << argv[4] << " (" << width << "x" << height << ").\\n";
        return 0;
    }
    if (command == "scene-render") {
        if (argc != 5 && argc != 7) { print_usage(); return 1; }
        std::uint32_t width = 640, height = 480;
        if (argc == 7 && (!parse_dimension(argv[5], width) ||
            !parse_dimension(argv[6], height) || width == 0 || height == 0)) {
            std::cerr << "error: width and height must be positive integers\\n";
            return 2;
        }
        const std::filesystem::path project_root(argv[2]);
        auto source = ecos::studio::platform::read_text_file(std::filesystem::path(argv[3]));
        if (!source) { std::cerr << "error: " << source.error().message << '\\n'; return 2; }
        auto scene = ecos::studio::deserialize_scene(source.value());
        if (!scene) { std::cerr << "error: " << scene.error().message << '\\n'; return 2; }
        auto assets = ecos::studio::load_imported_asset_registry(project_root);
        if (!assets) { std::cerr << "error: " << assets.error().message << '\\n'; return 2; }
        auto image = ecos::studio::render_scene_graph_to_ppm(
            scene.value(), assets.value(), project_root, width, height);
        if (!image) { std::cerr << "error: " << image.error().message << '\\n'; return 2; }
        std::ofstream output(std::filesystem::path(argv[4]), std::ios::binary);
        if (!output) { std::cerr << "error: cannot open output image for writing\\n"; return 2; }
        output.write(image.value().data(), static_cast<std::streamsize>(image.value().size()));
        if (!output) { std::cerr << "error: failed while writing output image\\n"; return 2; }
        std::cout << "Rendered scene with " << scene.value().size() << " node(s) to "
                  << argv[4] << " (" << width << "x" << height << ").\\n";
        return 0;
    }
    if (command == "mesh-render") {
        if (argc != 4 && argc != 6) { print_usage(); return 1; }
        std::uint32_t width=640, height=480;
        if (argc == 6 && (!parse_dimension(argv[4],width) || !parse_dimension(argv[5],height) || width == 0 || height == 0)) {
            std::cerr << "error: width and height must be positive integers\n"; return 2;
        }
        auto source=ecos::studio::platform::read_text_file(std::filesystem::path(argv[2]));
        if (!source) { std::cerr << "error: " << source.error().message << '\n'; return 2; }
        auto mesh=ecos::studio::import_obj_mesh(source.value());
        if (!mesh) { std::cerr << "error: " << mesh.error().message << '\n'; return 2; }
        auto image=ecos::studio::render_mesh_to_ppm(mesh.value(),width,height);
        if (!image) { std::cerr << "error: " << image.error().message << '\n'; return 2; }
        std::ofstream output(std::filesystem::path(argv[3]),std::ios::binary);
        if (!output) { std::cerr << "error: cannot open output image for writing\n"; return 2; }
        output.write(image.value().data(),static_cast<std::streamsize>(image.value().size()));
        if (!output) { std::cerr << "error: failed while writing output image\n"; return 2; }
        std::cout << "Rendered " << mesh.value().positions.size() << " vertices / "
                  << mesh.value().indices.size()/3 << " triangles to " << argv[3]
                  << " (" << width << "x" << height << ").\n";
        return 0;
    }
    std::cerr << "error: unknown command '" << command << "'\n";
    print_usage(); return 1;
}
