module;

#include <concepts>

export module rays:ray;

import :type;
import :vector;

namespace rays {

/// Ray.
export template <std::floating_point T> struct Ray {

    /// Origin.
    Vector<T, 3> origin;
    /// Direction.
    Vector<T, 3> direction;
};

export using Ray3f = Ray<Float>;

} // namespace rays
