module;

#include "math/math.hpp" // IWYU pragma: keep

export module rays:triangle;

import :ray;
import :type;
import :vector;

namespace rays {

/// Triangle.
export struct Triangle {

    /// Indices of vertices.
    UInt a, b, c;

    /// Test ray against triangle with given vertex positions.
    [[nodiscard]] bool Intersect(const Ray3f &ray, const Vector3f &v0,
                                 const Vector3f &v1,
                                 const Vector3f &v2) const noexcept {
        const Vector3f edge1 = v1 - v0;
        const Vector3f edge2 = v2 - v0;

        // Cross product of ray direction and edge2.
        const Vector3f pvec{
            ray.direction[1] * edge2[2] - ray.direction[2] * edge2[1],
            ray.direction[2] * edge2[0] - ray.direction[0] * edge2[2],
            ray.direction[0] * edge2[1] - ray.direction[1] * edge2[0]};

        // Determinant, zero means ray is parallel to triangle.
        const Float det = linalg::dot(edge1.View(), pvec.View());
        if (det == Float{0}) {
            return false;
        }
        const Float inv_det = Float{1} / det;

        // Barycentric coordinate u.
        const Vector3f tvec = ray.origin - v0;
        const Float u = linalg::dot(tvec.View(), pvec.View()) * inv_det;
        if (u < Float{0} || u > Float{1}) {
            return false;
        }

        // Barycentric coordinate v.
        const Vector3f qvec{tvec[1] * edge1[2] - tvec[2] * edge1[1],
            tvec[2] * edge1[0] - tvec[0] * edge1[2],
            tvec[0] * edge1[1] - tvec[1] * edge1[0]};
        const Float v = linalg::dot(ray.direction.View(), qvec.View()) * inv_det;
        if (v < Float{0} || u + v > Float{1}) {
            return false;
        }

        // Distance from ray origin to intersection.
        const Float t = linalg::dot(edge2.View(), qvec.View()) * inv_det;
        return t > Float{0};
    }
};

} // namespace rays
