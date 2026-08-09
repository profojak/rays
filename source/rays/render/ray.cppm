module;

#include <concepts>

export module rays:ray;

import :type;
import :vector;

namespace rays {

/// Forward declarations.
export struct Triangle;
export struct Sphere;

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

    Intersection(Float t, Vector2f uv) : t{t}, uv{uv} {}

    /// Distance along ray to intersection point.
    Float t;
    /// Barycentric UV coordinates of intersection point.
    Vector2f uv;
    /// Index of mesh containing intersection point.
    UInt mesh_index{0};
    /// Index of triangle containing intersection point.
    UInt triangle_index{0};
};

/// Sphere intersection specialization.
export template <std::floating_point T> struct Intersection<T, Sphere> {

    Intersection(Float t) : t{t} {}

    /// Distance along ray to intersection point.
    Float t;
};

export using Ray3f = Ray<Float>;
export using TriangleIntersection3f = Intersection<Float, Triangle>;
export using SphereIntersection3f = Intersection<Float, Sphere>;

} // namespace rays
