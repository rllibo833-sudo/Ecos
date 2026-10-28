#include "ecos_studio/mesh.hpp"

#include <cmath>
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
    auto mesh = import_obj_mesh(R"OBJ(# triangle
v -1 0 0
v 2 0 0
v 0 3 1
f 1 2 3
)OBJ");
    check(mesh, "imports a triangle OBJ");
    if (mesh) {
        check(mesh.value().positions.size() == 3, "stores vertex positions");
        check(mesh.value().indices.size() == 3, "creates one triangle");
        check(mesh.value().bounds.minimum.x == -1.0F, "computes minimum bounds");
        check(mesh.value().bounds.maximum.y == 3.0F, "computes maximum bounds");
        check(mesh.value().normals[0].z > 0.0F, "generates normals when OBJ omits them");
        check(std::abs(std::sqrt(mesh.value().normals[0].x*mesh.value().normals[0].x +
            mesh.value().normals[0].y*mesh.value().normals[0].y +
            mesh.value().normals[0].z*mesh.value().normals[0].z)-1.0F) < 1.0e-5F,
            "generated normals are normalized");
    }
}
void test_polygon_and_negative_indices() {
    using namespace ecos::studio;
    auto mesh = import_obj_mesh(R"OBJ(v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
f -4 -3 -2 -1
)OBJ");
    check(mesh, "imports polygon using negative indices");
    if (mesh) check(mesh.value().indices.size() == 6, "triangulates quad into two triangles");
}
void test_uv_normals_and_separate_indices() {
    using namespace ecos::studio;
    const std::string obj = R"OBJ(v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
vt 0 0
vt 1 0
vt 1 1
vt 0 1
vn 0 0 1
f 1/1/1 2/2/1 3/3/1
f 1/4/1 3/3/1 4/1/1
)OBJ";
    auto mesh=import_obj_mesh(obj);
    check(mesh, "imports UVs and normals with independent OBJ indices");
    if (mesh) {
        check(mesh.value().indices.size()==6, "keeps both textured triangles");
        check(mesh.value().positions.size()==5, "duplicates seam vertex when UV differs");
        check(mesh.value().texcoords.size()==mesh.value().positions.size(), "UVs align with expanded vertices");
        check(mesh.value().normals.size()==mesh.value().positions.size(), "normals align with expanded vertices");
        bool uv_found=false;
        for (const auto& uv:mesh.value().texcoords) if (uv.x==0.0F && uv.y==1.0F) uv_found=true;
        check(uv_found, "preserves texture seam coordinates");
        check(mesh.value().normals[0].z==1.0F, "preserves imported normal");
    }
}
void test_invalid_meshes() {
    using namespace ecos::studio;
    check(!import_obj_mesh(R"OBJ(v 0 0 0
f 1 2 3
)OBJ"), "rejects face references to missing vertices");
    check(!import_obj_mesh(R"OBJ(v 0 0 0
v nan 1 2
f 1 1 1
)OBJ"), "rejects non-finite coordinates");
    check(!import_obj_mesh(R"OBJ(v 0 0 0
v 1 0 0
v 0 1 0
)OBJ"), "rejects meshes without faces");
    check(!import_obj_mesh(R"OBJ(v 0 0 0
v 1 0 0
v 0 1 0
vt 0 0
f 1/9 2/1 3/1
)OBJ"), "rejects out-of-range UV indices");
    check(!import_obj_mesh(R"OBJ(v 0 0 0
v 1 0 0
v 0 1 0
vn 0 0 1
f 1//3 2//1 3//1
)OBJ"), "rejects out-of-range normal indices");
    check(!import_obj_mesh(R"OBJ(v 0 0 0
v 1 0 0
v 0 1 0
f 1///1 2 3
)OBJ"), "rejects malformed face corner with extra slash");
}
}
int main() {
    test_triangle_and_bounds();
    test_polygon_and_negative_indices();
    test_uv_normals_and_separate_indices();
    test_invalid_meshes();
    if (failures != 0) { std::cerr << failures << " test(s) failed.\n"; return 1; }
    std::cout << "ECOS Studio mesh import tests passed.\n";
    return 0;
}
