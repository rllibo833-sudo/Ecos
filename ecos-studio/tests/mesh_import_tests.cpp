#include "ecos_studio/mesh.hpp"

#include <iostream>
#include <string>

namespace {
int failures = 0;
template <typename T>
void check(const T& condition, const char* message) {
    if (!static_cast<bool>(condition)) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
void test_triangle_and_bounds() {
    using namespace ecos::studio;
    const std::string obj =
        "# triangle\n"
        "v -1 0 0\n"
        "v 2 0 0\n"
        "v 0 3 1\n"
        "f 1 2 3\n";
    auto mesh = import_obj_mesh(obj);
    check(mesh, "imports a triangle OBJ");
    if (mesh) {
        check(mesh.value().positions.size() == 3, "stores vertex positions");
        check(mesh.value().indices.size() == 3, "creates one triangle");
        check(mesh.value().bounds.minimum.x == -1.0F, "computes minimum bounds");
        check(mesh.value().bounds.maximum.y == 3.0F, "computes maximum bounds");
    }
}
void test_polygon_and_negative_indices() {
    using namespace ecos::studio;
    const std::string obj =
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "f -4 -3 -2 -1\n";
    auto mesh = import_obj_mesh(obj);
    check(mesh, "imports polygon using negative indices");
    if (mesh) check(mesh.value().indices.size() == 6, "triangulates quad into two triangles");
}
void test_invalid_meshes() {
    using namespace ecos::studio;
    check(!import_obj_mesh("v 0 0 0\nf 1 2 3\n"), "rejects face references to missing vertices");
    check(!import_obj_mesh("v 0 0 0\nv nan 1 2\nf 1 1 1\n"), "rejects non-finite coordinates");
    check(!import_obj_mesh("v 0 0 0\nv 1 0 0\nv 0 1 0\n"), "rejects meshes without faces");
}
}
int main() {
    test_triangle_and_bounds();
    test_polygon_and_negative_indices();
    test_invalid_meshes();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio mesh import tests passed.\n";
    return 0;
}
