#include "ecos_studio/render_backend.hpp"
#include "ecos_studio/mesh.hpp"

#include <iostream>
#include <limits>

namespace {
int failures = 0;
void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}
}

int main() {
    using namespace ecos::studio;
    CpuReferenceRenderBackend backend;
    const auto metadata = backend.info();
    check(metadata.name == "cpu-reference", "identifies the CPU reference backend");
    check(!metadata.hardware_accelerated, "does not claim hardware acceleration");
    check(metadata.supports_depth_buffer, "advertises depth-buffer support");
    check(metadata.supports_clip_space_clipping, "advertises clip-space clipping");

    auto mesh = import_obj_mesh(
        "v -0.7 -0.6 0\nv 0.7 -0.6 0\nv 0 0.7 0\nf 1 2 3\n");
    check(static_cast<bool>(mesh), "imports backend contract test triangle");
    if (mesh) {
        const std::vector<RenderMeshInstance> instances{
            RenderMeshInstance{&mesh.value(), Matrix4::identity()}};
        const auto expected = render_scene_to_ppm(
            instances, Matrix4::identity(), 64, 64);
        const auto actual = backend.render_frame(
            instances, Matrix4::identity(), 64, 64);
        check(expected && actual, "renders frame through backend contract");
        if (expected && actual)
            check(expected.value() == actual.value(),
                "CPU backend output matches the deterministic reference renderer");
        check(!backend.render_frame({}, Matrix4::identity(), 64, 64),
            "propagates invalid empty-scene errors");
        check(!backend.render_frame(instances, Matrix4::identity(), 0, 64),
            "propagates invalid frame-dimension errors");
        auto invalid_matrix = Matrix4::identity();
        invalid_matrix.values[0] = std::numeric_limits<float>::quiet_NaN();
        check(!backend.render_frame(instances, invalid_matrix, 64, 64),
            "propagates invalid camera-matrix errors");

        auto red_instances = instances;
        red_instances[0].diffuse_color = {1.0F, 0.0F, 0.0F};
        auto blue_instances = instances;
        blue_instances[0].diffuse_color = {0.0F, 0.0F, 1.0F};
        const auto red_frame = backend.render_frame(
            red_instances, Matrix4::identity(), 64, 64);
        const auto blue_frame = backend.render_frame(
            blue_instances, Matrix4::identity(), 64, 64);
        check(red_frame && blue_frame && red_frame.value() != blue_frame.value(),
            "different material diffuse colors produce different frame pixels");

        auto invalid_material = instances;
        invalid_material[0].diffuse_color = {1.2F, 0.0F, 0.0F};
        check(!backend.render_frame(invalid_material, Matrix4::identity(), 64, 64),
            "rejects out-of-range material color");
        invalid_material[0].diffuse_color = {
            std::numeric_limits<float>::quiet_NaN(), 0.0F, 0.0F};
        check(!backend.render_frame(invalid_material, Matrix4::identity(), 64, 64),
            "rejects non-finite material color");
    }

    if (failures) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "ECOS Studio render backend contract tests passed.\n";
    return 0;
}
