module;

#include <concepts>

export module rays:ray;

import :type;
import :vector;

namespace rays {

/// Forward declaration of `Triangle`.
export struct Triangle;

/// Ray.
export template <std::floating_point T> struct Ray {

    /// Origin.
    Vector<T, 3> origin;
    /// Direction.
    Vector<T, 3> direction;
};

/// Generic ray-surface intersection.
export template <std::floating_point T, typename Surface>
struct Intersection {};

/// Triangle intersection specialization.
export template <std::floating_point T> struct Intersection<T, Triangle> {

    /// Distance along ray to intersection point.
    Float t;
    /// Barycentric UV coordinates of intersection point.
    Vector2f uv;
};

export using Ray3f = Ray<Float>;
export using TriangleIntersection3f = Intersection<Float, Triangle>;

} // namespace rays
