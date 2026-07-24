#include "BoundingCube.h"
#include "Matrix.h"
#include "Vector.h"
#include <iostream>

namespace Argon {
bool BoundingCube::overlaps_point(Vector3f point) const {
    if (point[0] < origin[0])
        return false;
    if (point[1] < origin[1])
        return false;
    if (point[2] < origin[2])
        return false;

    if (point[0] > origin[0] + size[0])
        return false;
    if (point[1] > origin[1] + size[1])
        return false;
    if (point[2] > origin[2] + size[2])
        return false;
    return true;
}
bool BoundingCube::overlaps_point(Vector2f point) const {
    if (point[0] < origin[0])
        return false;
    if (point[1] < origin[1])
        return false;

    if (point[0] > origin[0] + size[0])
        return false;
    if (point[1] > origin[1] + size[1])
        return false;
    return true;
}
void BoundingCube::add_point_to_bounds(const VectorBase<float, 3>& point) {
    if (size[0] == -1.) {
        origin = point;
        size.set_all(0);
        return;
    }

    const Vector3f t = point - origin;
    if (t[0] < 0.0f) {
        size[0] -= t[0];
        origin[0] = point[0];
    } else if (t[0] > size[0])
        size[0] = t[0];
    if (t[1] < 0.0f) {
        size[1] -= t[1];
        origin[1] = point[1];
    } else if (t[1] > size[1])
        size[1] = t[1];
    if (t[2] < 0.0f) {
        size[2] -= t[2];
        origin[2] = point[2];
    } else if (t[2] > size[2])
        size[2] = t[2];
}
Vector3f BoundingCube::get_positive_vertex(Vector3f normal) const {
    return Vector3f(normal[0] < 0.0 ? origin[0] : origin[0] + size[0],
                    normal[1] < 0.0 ? origin[1] : origin[1] + size[1],
                    normal[2] < 0.0 ? origin[2] : origin[2] + size[2]);
}
Vector3f BoundingCube::get_negative_vertex(Vector3f normal) const {
    return Vector3f(normal[0] < 0.0 ? origin[0] + size[0] : origin[0],
                    normal[1] < 0.0 ? origin[1] + size[1] : origin[1],
                    normal[2] < 0.0 ? origin[2] + size[2] : origin[2]);
}

BoundingCube BoundingCube::transform(const Matrix4f& aff) {
    BoundingCube cube;
    Matrix4f m = aff;
    Vector4f a(origin[0], origin[1], origin[2], 1.);
    Vector4f b(origin[0] + size[0], origin[1], origin[2], 1.);
    Vector4f c(origin[0] + size[0], origin[1] + size[1], origin[2], 1.);
    Vector4f d(origin[0], origin[1] + size[1], origin[2], 1.);

    Vector4f e(origin[0], origin[1], origin[2] + size[2], 1.);
    Vector4f f(origin[0] + size[0], origin[1], origin[2] + size[2], 1.);
    Vector4f g(origin[0] + size[0], origin[1] + size[1], origin[2] + size[2],
               1.);
    Vector4f h(origin[0], origin[1] + size[1], origin[2] + size[2], 1.);

    cube.add_point_to_bounds((m * a));
    cube.add_point_to_bounds((m * b));
    cube.add_point_to_bounds((m * c));
    cube.add_point_to_bounds((m * d));
    cube.add_point_to_bounds((m * e));
    cube.add_point_to_bounds((m * f));
    cube.add_point_to_bounds((m * g));
    cube.add_point_to_bounds((m * h));

    return cube;
} // transform
MAKE_VISIT_IMPL(BoundingCube, {
    ADD_VAR(origin);
    ADD_VAR(size);
});
} // namespace Argon
