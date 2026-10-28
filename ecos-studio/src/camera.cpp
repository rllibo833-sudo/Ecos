#include "ecos_studio/camera.hpp"

#include <algorithm>
#include <cmath>

namespace ecos::studio {
namespace {
Vec3 subtract(Vec3 a, Vec3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
float dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
float magnitude(Vec3 v) { return std::sqrt(dot(v,v)); }
bool finite(Vec3 v) { return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z); }
bool finite_matrix(const Matrix4& m) {
    return std::all_of(m.values.begin(),m.values.end(),[](float v){return std::isfinite(v);});
}
}
Result<Matrix4> look_at(Vec3 eye, Vec3 target, Vec3 up) {
    if (!finite(eye)||!finite(target)||!finite(up))
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Camera vectors must be finite"});
    const Vec3 direction=subtract(target,eye);
    const float direction_length=magnitude(direction);
    if (direction_length<=1.0e-7F)
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Camera eye and target must differ"});
    const Vec3 forward{direction.x/direction_length,direction.y/direction_length,direction.z/direction_length};
    const Vec3 side_raw=cross(forward,up);
    const float side_length=magnitude(side_raw);
    if (side_length<=1.0e-7F)
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Camera up vector must not be parallel to view direction"});
    const Vec3 side{side_raw.x/side_length,side_raw.y/side_length,side_raw.z/side_length};
    const Vec3 corrected_up=cross(side,forward);
    Matrix4 result;
    result.values={
        side.x,side.y,side.z,-dot(side,eye),
        corrected_up.x,corrected_up.y,corrected_up.z,-dot(corrected_up,eye),
        -forward.x,-forward.y,-forward.z,dot(forward,eye),
        0.0F,0.0F,0.0F,1.0F
    };
    return Result<Matrix4>::success(result);
}
Result<Matrix4> perspective(float fov, float aspect, float near_plane, float far_plane) {
    constexpr float pi=3.14159265358979323846F;
    if (!std::isfinite(fov)||!std::isfinite(aspect)||!std::isfinite(near_plane)||!std::isfinite(far_plane)||
        fov<=0.0F||fov>=pi||aspect<=0.0F||near_plane<=0.0F||far_plane<=near_plane)
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Perspective requires 0 < FOV < pi, aspect > 0, and 0 < near < far"});
    const float tangent=std::tan(fov*0.5F);
    if (!std::isfinite(tangent)||tangent<=0.0F)
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Perspective field of view is numerically invalid"});
    const float f=1.0F/tangent;
    Matrix4 result;
    result.values={
        f/aspect,0.0F,0.0F,0.0F,
        0.0F,f,0.0F,0.0F,
        0.0F,0.0F,(far_plane+near_plane)/(near_plane-far_plane),
        (2.0F*far_plane*near_plane)/(near_plane-far_plane),
        0.0F,0.0F,-1.0F,0.0F
    };
    if (!finite_matrix(result))
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Perspective parameters exceed numeric range"});
    return Result<Matrix4>::success(result);
}
Result<Matrix4> orthographic(float left, float right, float bottom, float top,
                             float near_plane, float far_plane) {
    if (!std::isfinite(left)||!std::isfinite(right)||!std::isfinite(bottom)||!std::isfinite(top)||
        !std::isfinite(near_plane)||!std::isfinite(far_plane)||right==left||top==bottom||far_plane==near_plane)
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Orthographic bounds must be finite and non-degenerate"});
    Matrix4 result;
    result.values={
        2.0F/(right-left),0.0F,0.0F,-(right+left)/(right-left),
        0.0F,2.0F/(top-bottom),0.0F,-(top+bottom)/(top-bottom),
        0.0F,0.0F,-2.0F/(far_plane-near_plane),-(far_plane+near_plane)/(far_plane-near_plane),
        0.0F,0.0F,0.0F,1.0F
    };
    if (!finite_matrix(result))
        return Result<Matrix4>::failure({ErrorCode::invalid_argument,"Orthographic bounds exceed numeric range"});
    return Result<Matrix4>::success(result);
}
Vec4 transform(const Matrix4& matrix, Vec4 v) noexcept {
    return {
        matrix.values[0]*v.x+matrix.values[1]*v.y+matrix.values[2]*v.z+matrix.values[3]*v.w,
        matrix.values[4]*v.x+matrix.values[5]*v.y+matrix.values[6]*v.z+matrix.values[7]*v.w,
        matrix.values[8]*v.x+matrix.values[9]*v.y+matrix.values[10]*v.z+matrix.values[11]*v.w,
        matrix.values[12]*v.x+matrix.values[13]*v.y+matrix.values[14]*v.z+matrix.values[15]*v.w
    };
}
Result<Vec3> project_to_ndc(const Matrix4& matrix, Vec3 position) {
    if (!finite(position)||!finite_matrix(matrix))
        return Result<Vec3>::failure({ErrorCode::invalid_argument,"Projection inputs must be finite"});
    const Vec4 clip=transform(matrix,{position.x,position.y,position.z,1.0F});
    if (!std::isfinite(clip.w)||std::abs(clip.w)<=1.0e-7F)
        return Result<Vec3>::failure({ErrorCode::invalid_argument,"Point cannot be projected with a zero or invalid homogeneous W"});
    const Vec3 result{clip.x/clip.w,clip.y/clip.w,clip.z/clip.w};
    if (!finite(result))
        return Result<Vec3>::failure({ErrorCode::invalid_argument,"Projected point exceeds numeric range"});
    return Result<Vec3>::success(result);
}
} // namespace ecos::studio
