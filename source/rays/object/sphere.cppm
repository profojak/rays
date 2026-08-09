module;

#include "math/math.hpp" // IWYU pragma: keep

#include <cmath>
#include <optional>

export module rays:sphere;

import :point;
import :ray;
import :type;
import :vector;

namespace rays {

/// Sphere.
export struct Sphere {

    /// Center of sphere.
    Point3f center;
    /// Radius of sphere.
    float radius;

    /// Test ray against sphere.
    [[nodiscard]] std::optional<SphereIntersection3f>
    Intersect(const Ray3f &ray) const noexcept {
        const Vector3f oc = ray.origin - center;

        const Float a = linalg::dot(ray.direction.View(), ray.direction.View());
        const Float b = linalg::dot(oc.View(), ray.direction.View());
        const Float c = linalg::dot(oc.View(), oc.View()) - radius * radius;

        if (a == 0.0f) {
            return std::nullopt;
        }
        const Float discriminant = b * b - a * c;
        if (discriminant < 0.0f) {
            return std::nullopt;
        }
        const Float sqrt_discriminant = std::sqrt(discriminant);

        Float t = (-b - sqrt_discriminant) / a;
        if (t <= 0.0f) {
            t = (-b + sqrt_discriminant) / a;
        }
        if (t <= 0.0f) {
            return std::nullopt;
        }

        return SphereIntersection3f{t};
    }
};

} // namespace rays
