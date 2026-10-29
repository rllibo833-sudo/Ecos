#include "ecos_studio/mesh.hpp"
#include "ecos_studio/render.hpp"
#include "ecos_studio/asset_registry.hpp"
#include "ecos_studio/world_content_runtime.hpp"
#include "ecos_studio/world_manifest.hpp"
#include "ecos_studio/job_system.hpp"
#include "ecos_studio/camera.hpp"
#include "ecos_studio/engine_loop.hpp"

#include <cstdint>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

namespace {
int failures=0;
void check(bool condition,const char* message) {
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_renders_triangle() {
    using namespace ecos::studio;
    auto mesh=import_obj_mesh("v -1 -1 0\nv 1 -1 0\nv 0 1 0\nf 1 2 3\n");
    check(static_cast<bool>(mesh),"test mesh imports");
    if (!mesh) return;
    auto image=render_mesh_to_ppm(mesh.value(),64,64);
    check(static_cast<bool>(image),"triangle renders");
    if (!image) return;
    check(image.value().starts_with("P6\n64 64\n255\n"),"writes PPM header");
    const auto header_end=image.value().find("255\n")+4;
    bool found_geometry=false;
    for (std::size_t i=header_end;i+2<image.value().size();i+=3) {
        const auto r=static_cast<std::uint8_t>(image.value()[i]);
        const auto g=static_cast<std::uint8_t>(image.value()[i+1]);
        const auto b=static_cast<std::uint8_t>(image.value()[i+2]);
        if (r!=18 || g!=22 || b!=30) { found_geometry=true; break; }
    }
    check(found_geometry,"rasterizes colored triangle pixels");
    check(image.value().size()==header_end+64U*64U*3U,"image contains exact RGB payload");
}

void test_world_transform_changes_rendered_geometry() {
    using namespace ecos::studio;
    auto mesh = import_obj_mesh("v -1 -1 0\nv 1 -1 0\nv 0 1 0\nf 1 2 3\n");
    check(static_cast<bool>(mesh), "mesh imports for world-transform render test");
    if (!mesh) return;
    const auto identity = Matrix4::identity();
    auto baseline = render_mesh_to_ppm(mesh.value(), identity, 64, 64);
    Transform transform;
    transform.scale = {1.0F, 0.25F, 1.0F};
    const auto flattened = compose(transform);
    auto transformed = render_mesh_to_ppm(mesh.value(), flattened, 64, 64);
    check(baseline && transformed, "renders mesh with identity and world transforms");
    if (baseline && transformed) check(baseline.value() != transformed.value(),
        "world transform affects rasterized geometry");
    auto invalid = Matrix4::identity();
    invalid.values[0] = std::numeric_limits<float>::infinity();
    check(!render_mesh_to_ppm(mesh.value(), invalid, 64, 64),
        "rejects non-finite world transform matrices");
}

void test_renders_multiple_transformed_instances_in_shared_scene() {
    using namespace ecos::studio;
    auto mesh = import_obj_mesh("v -0.4 -0.4 0\nv 0.4 -0.4 0\nv 0 0.4 0\nf 1 2 3\n");
    check(static_cast<bool>(mesh), "mesh imports for multi-instance scene");
    if (!mesh) return;
    const auto identity = Matrix4::identity();
    Transform offset;
    offset.position = {1.5F, 0.0F, 0.0F};
    const auto translated = compose(offset);
    auto single = render_scene_to_ppm({RenderMeshInstance{&mesh.value(), identity}}, 96, 64);
    auto multiple = render_scene_to_ppm({
        RenderMeshInstance{&mesh.value(), identity},
        RenderMeshInstance{&mesh.value(), translated}}, 96, 64);
    check(single && multiple, "renders one and multiple mesh instances");
    if (single && multiple) check(single.value() != multiple.value(),
        "second object's world transform changes shared scene output");
    check(!render_scene_to_ppm({}, 32, 32), "rejects empty render scene");
    check(!render_scene_to_ppm({RenderMeshInstance{nullptr, identity}}, 32, 32),
        "rejects null mesh instance");
    auto invalid = identity;
    invalid.values[12] = std::numeric_limits<float>::quiet_NaN();
    check(!render_scene_to_ppm({RenderMeshInstance{&mesh.value(), invalid}}, 32, 32),
        "rejects non-finite instance transform");
}

void test_scene_graph_resolves_mesh_assets_and_world_transforms() {
    using namespace ecos::studio;
    const auto root = std::filesystem::temp_directory_path() / "ecos-scene-render-assets";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "assets");
    {
        std::ofstream out(root / "assets/triangle.obj", std::ios::binary);
        out << "v -0.4 -0.4 0\nv 0.4 -0.4 0\nv 0 0.4 0\nf 1 2 3\n";
    }
    AssetRegistry registry;
    check(static_cast<bool>(registry.register_asset(AssetRecord{"triangle", "mesh.obj", "assets/triangle.obj", {}})),
        "registers mesh asset for scene renderer");
    SceneGraph scene;
    auto left = scene.create_node("Left");
    auto right = scene.create_node("Right", left.value());
    check(left && right, "creates graph for asset rendering");
    if (left && right) {
        check(static_cast<bool>(scene.set_mesh_asset(left.value(), "triangle")), "assigns root mesh asset");
        check(static_cast<bool>(scene.set_mesh_asset(right.value(), "triangle")), "assigns child mesh asset");
        Transform transform;
        transform.position = {1.0F, 0.0F, 0.0F};
        check(static_cast<bool>(scene.set_local_transform(right.value(), transform)), "transforms child mesh");
        auto image = render_scene_graph_to_ppm(scene, registry, root, 96, 64);
        check(static_cast<bool>(image), "loads referenced OBJ assets and renders scene graph");
        if (image) check(image.value().starts_with("P6\n96 64\n255\n"), "scene graph renderer returns a PPM frame");
    }
    SceneGraph missing_scene;
    auto missing_node = missing_scene.create_node("Missing");
    if (missing_node) check(static_cast<bool>(missing_scene.set_mesh_asset(missing_node.value(), "absent")), "sets missing asset for validation");
    check(!render_scene_graph_to_ppm(missing_scene, registry, root, 32, 32), "rejects unresolved scene mesh asset references");
    std::filesystem::remove_all(root);
}


void test_renders_only_resident_streamed_world_cells() {
    using namespace ecos::studio;
    const auto root = std::filesystem::temp_directory_path() / "ecos-streamed-world-render";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "assets");
    {
        std::ofstream out(root / "assets/triangle.obj", std::ios::binary);
        out << "v -0.4 -0.4 0\nv 0.4 -0.4 0\nv 0 0.4 0\nf 1 2 3\n";
    }
    {
        std::ofstream out(root / "world.ecworld", std::ios::binary);
        out << "format_version=1\nworld_id=render-test\n"
               "cell=origin|region|0,0,0|cells/origin.ecell\n"
               "cell=east|region|10,0,0|cells/east.ecell\n";
    }
    std::filesystem::create_directories(root / "cells");
    for (const auto* cell : {"origin", "east"}) {
        std::ofstream out(root / "cells" / (std::string(cell) + ".ecell"), std::ios::binary);
        out << "format_version=2\nscene_id=" << cell
            << "\nnode=1|-|Mesh|0,0,0|0,0,0|1,1,1|triangle\n";
    }

    auto manifest = load_world_manifest(root / "world.ecworld");
    check(static_cast<bool>(manifest), "loads streamed-render world manifest");
    if (manifest) {
        JobSystem jobs(2);
        auto created = WorldContentRuntime::create(std::move(manifest.value()), jobs, 2);
        check(static_cast<bool>(created), "creates streamed-render runtime");
        if (created) {
            auto runtime = std::move(created.value());
            auto started = runtime->update({0, 0, 0}, 10, 10);
            check(started && started.value().started_loads == 2, "schedules both nearby scene cells");
            jobs.wait_idle();
            auto completed = runtime->update({0, 0, 0}, 10, 10);
            check(completed && completed.value().completed_loads == 2, "loads both streamed scene cells");
            AssetRegistry registry;
            check(static_cast<bool>(registry.register_asset(
                AssetRecord{"triangle", "mesh.obj", "assets/triangle.obj", {}})),
                "registers shared mesh used by streamed cells");
            auto combined = render_world_content_runtime_to_ppm(*runtime, registry, root, 96, 64);
            check(static_cast<bool>(combined), "renders all resident world cells into one frame");
            auto camera_view = look_at({5.0F, 0.0F, 20.0F}, {5.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F});
            auto camera_projection = perspective(1.04719755F, 1.5F, 0.1F, 100.0F);
            check(camera_view && camera_projection, "builds camera matrices for streamed world");
            if (camera_view && camera_projection) {
                auto camera_frame = render_world_content_runtime_to_ppm(*runtime, registry, root,
                    multiply(camera_projection.value(), camera_view.value()), 96, 64);
                check(static_cast<bool>(camera_frame), "renders resident streamed world through active camera");
                if (camera_frame) check(camera_frame.value().starts_with("P6\\n96 64\\n255\\n"),
                    "camera-aware streamed renderer emits a valid frame");
            }
            if (combined) check(combined.value().starts_with("P6\n96 64\n255\n"),
                "streamed world renderer returns a PPM frame");

            auto unload = runtime->update({0, 0, 0}, 0, 0);
            check(unload && unload.value().unloaded_cells == 1, "unloads the distant cell");
            auto resident_only = render_world_content_runtime_to_ppm(*runtime, registry, root, 96, 64);
            check(static_cast<bool>(resident_only), "renders the remaining resident cell");
            if (combined && resident_only) check(combined.value() != resident_only.value(),
                "unloading a cell removes its geometry from subsequent frames");
        }
    }
    std::filesystem::remove_all(root);
}


void test_camera_projection_and_engine_frame_integration() {
    using namespace ecos::studio;
    auto mesh=import_obj_mesh("v -0.7 -0.6 0\nv 0.7 -0.6 0\nv 0 0.7 0\nf 1 2 3\n");
    check(static_cast<bool>(mesh), "imports triangle for camera renderer");
    if (!mesh) return;
    auto view=look_at({0,0,3},{0,0,0},{0,1,0});
    auto projection=perspective(1.04719755F,1.5F,0.1F,100.0F);
    check(view&&projection,"creates view and perspective matrices");
    if (!view||!projection) return;
    const auto vp=multiply(projection.value(),view.value());
    auto image=render_scene_to_ppm({RenderMeshInstance{&mesh.value(),Matrix4::identity()}},vp,96,64);
    check(static_cast<bool>(image),"renders a mesh through the camera view-projection matrix");
    if (image) {
        check(image.value().starts_with("P6\n96 64\n255\n"),"camera renderer emits a valid PPM header");
        const auto offset=image.value().find("255\n")+4;
        bool geometry=false;
        for(std::size_t i=offset;i+2<image.value().size();i+=3) {
            const auto r=static_cast<std::uint8_t>(image.value()[i]);
            const auto g=static_cast<std::uint8_t>(image.value()[i+1]);
            const auto b=static_cast<std::uint8_t>(image.value()[i+2]);
            if(r!=18||g!=22||b!=30){geometry=true;break;}
        }
        check(geometry,"camera projection rasterizes visible geometry");
    }
    auto shifted_view=look_at({1.0F,0.0F,3.0F},{0,0,0},{0,1,0});
    if (shifted_view) {
        auto shifted=render_scene_to_ppm({RenderMeshInstance{&mesh.value(),Matrix4::identity()}},
            multiply(projection.value(),shifted_view.value()),96,64);
        check(shifted&&image&&shifted.value()!=image.value(),
            "moving the camera changes the rendered frame");
    }
    EngineLoop loop;
    bool callback_rendered=false;
    FrameCallbacks callbacks;
    callbacks.render=[&](double,const InputSnapshot&) {
        auto frame=render_scene_to_ppm({RenderMeshInstance{&mesh.value(),Matrix4::identity()}},
            vp,96,64);
        callback_rendered=static_cast<bool>(frame);
    };
    const auto report=loop.frame(1.0/60.0,callbacks);
    check(report.rendered&&callback_rendered,
        "engine frame render callback produces a camera-projected world frame");
}

void test_rejects_bad_inputs() {
    using namespace ecos::studio;
    MeshData mesh;
    check(!render_mesh_to_ppm(mesh,32,32),"rejects empty mesh");
    auto valid=import_obj_mesh("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
    if (valid) {
        check(!render_mesh_to_ppm(valid.value(),0,32),"rejects zero width");
        check(!render_mesh_to_ppm(valid.value(),5000,32),"rejects oversized image");
        valid.value().indices[0]=999;
        check(!render_mesh_to_ppm(valid.value(),32,32),"rejects invalid indices");
    }
}
}
int main() {
    test_renders_triangle();
    test_world_transform_changes_rendered_geometry();
    test_renders_multiple_transformed_instances_in_shared_scene();
    test_rejects_bad_inputs();
    test_scene_graph_resolves_mesh_assets_and_world_transforms();
    test_renders_only_resident_streamed_world_cells();
    test_camera_projection_and_engine_frame_integration();
    if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio CPU renderer tests passed.\n";
    return 0;
}
