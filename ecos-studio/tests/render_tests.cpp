#include "ecos_studio/mesh.hpp"
#include "ecos_studio/render.hpp"

#include <cstdint>
#include <iostream>
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
    test_rejects_bad_inputs();
    if (failures) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio CPU renderer tests passed.\n";
    return 0;
}
