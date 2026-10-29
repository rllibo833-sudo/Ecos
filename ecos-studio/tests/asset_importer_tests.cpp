#include "ecos_studio/asset_importer.hpp"
#include "ecos_studio/content_hash.hpp"
#include "ecos_studio/project.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
std::string read(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
void write(const fs::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << contents;
}
}

int main() {
    const auto root = fs::temp_directory_path() / "ecos-studio-asset-importer-tests";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root, ec);
    const auto project_root = root / "project";
    const auto source_path = root / "triangle.obj";
    std::string error;
    check(ecos::studio::create_project(project_root, "Importer Test", error), "create valid project scaffold");

    const std::string source =
        "# triangle mesh\n"
        "v -1 0 0\n"
        "v 2 0 0\n"
        "v 0 3 1\n"
        "f 1 2 3\n";
    write(source_path, source);

    auto imported = ecos::studio::import_obj_asset(project_root, source_path, "demo.triangle");
    check(static_cast<bool>(imported), "imports valid OBJ into project");
    if (imported) {
        check(imported.value().vertex_count == 3, "reports imported vertex count");
        check(imported.value().triangle_count == 1, "reports imported triangle count");
        check(imported.value().content_sha256 == ecos::studio::sha256_hex(source), "records SHA-256 of exact source bytes");
        const auto stored = project_root / "assets/imported/demo.triangle.obj";
        const auto metadata = project_root / "assets/imported/demo.triangle.ecasset";
        check(read(stored) == source, "preserves source bytes in project");
        const auto sidecar = read(metadata);
        check(sidecar.find("type=mesh.obj") != std::string::npos, "writes typed sidecar metadata");
        check(sidecar.find("content_sha256=" + imported.value().content_sha256) != std::string::npos, "sidecar digest matches import");
        check(sidecar.find("triangle_count=1") != std::string::npos, "sidecar stores triangle count");
        const auto validated = ecos::studio::validate_imported_assets(project_root);
        check(validated.ok, "validates freshly imported asset and metadata");
    }

    const auto mtl_path = root / "surface.mtl";
    const std::string mtl = "newmtl stone\nKd 0.4 0.5 0.6\nNs 24\nnewmtl glass\nd 0.4\n";
    write(mtl_path, mtl);
    auto imported_materials = ecos::studio::import_mtl_asset(project_root, mtl_path, "demo.materials");
    check(static_cast<bool>(imported_materials), "imports valid MTL material library");
    if (imported_materials) {
        check(imported_materials.value().material_count == 2, "reports material library count");
        check(read(project_root / "assets/imported/demo.materials.mtl") == mtl, "preserves MTL source bytes");
        check(read(project_root / "assets/imported/demo.materials.ecasset").find("type=material.mtl") != std::string::npos,
              "writes typed material asset metadata");
        check(ecos::studio::validate_imported_assets(project_root).ok, "validates mesh and material assets together");
    }
    auto duplicate_materials = ecos::studio::import_mtl_asset(project_root, mtl_path, "demo.materials");
    check(!duplicate_materials, "refuses to overwrite an existing MTL import");
    write(mtl_path, "newmtl invalid\nKd 2 0 0\n");
    check(!ecos::studio::import_mtl_asset(project_root, mtl_path, "invalid.material"),
          "rejects malformed MTL without creating an asset");

    auto duplicate = ecos::studio::import_obj_asset(project_root, source_path, "demo.triangle");
    check(!duplicate, "refuses to overwrite an existing import");
    check(!ecos::studio::import_obj_asset(project_root, source_path, "../escape"), "rejects unsafe asset IDs");

    write(source_path, "v 0 0 0\nf 1 2 3\n");
    auto invalid = ecos::studio::import_obj_asset(project_root, source_path, "invalid.mesh");
    check(!invalid, "rejects malformed OBJ without creating an asset");

    const auto stored = project_root / "assets/imported/demo.triangle.obj";
    write(stored, source + "# tampered\n");
    const auto tampered = ecos::studio::validate_imported_assets(project_root);
    check(!tampered.ok, "detects source bytes changed after import");
    write(stored, source);

    const auto metadata = project_root / "assets/imported/demo.triangle.ecasset";
    auto sidecar = read(metadata);
    const auto source_marker = sidecar.find("source=assets/imported/demo.triangle.obj");
    if (source_marker != std::string::npos)
        sidecar.replace(source_marker, std::string("source=assets/imported/demo.triangle.obj").size(), "source=../../outside.obj");
    write(metadata, sidecar);
    const auto escaped = ecos::studio::validate_imported_assets(project_root);
    check(!escaped.ok, "rejects metadata path traversal");

    auto missing_project = ecos::studio::import_obj_asset(root / "missing-project", source_path, "valid.id");
    check(!missing_project, "rejects destination that is not a valid project");

    fs::remove_all(root, ec);
    if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio asset importer tests passed.\n";
    return 0;
}
