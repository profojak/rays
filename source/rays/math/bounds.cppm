module;

#include <algorithm>
#include <cstddef>

export module rays:bounds;

import :point;
import :type;
import :vector;

namespace rays {

/// Bounds.
export template <arithmetic T, std::size_t N>
    requires(N == 2 || N == 3)
struct Bounds {

    /// Minimum point.
    Point<T, N> min;
    /// Maximum point.
    Point<T, N> max;

    /// Construct from size.
    template <arithmetic U>
    Bounds(const U size) : min{Point<T, N>{0}}, max{Point<T, N>{size}} {}

    /// Construct from minimum point and size.
    template <arithmetic U, arithmetic V>
    Bounds(const Point<U, N> &min, const V size)
        : min{min}, max{min + Point<V, N>{size}} {}

    /// Construct from scalar dimensions.
    template <arithmetic... Us>
        requires(sizeof...(Us) == N)
    Bounds(const Us... size) : min{Point<T, N>{0}}, max{Point<T, N>{size...}} {}

    /// Construct from minimum point and scalar dimensions.
    template <arithmetic... Us>
        requires(sizeof...(Us) == N)
    Bounds(const Point<T, N> &min, const Us... size)
        : min{min}, max{min + Point<T, N>{size...}} {}

    /// Return size along given dimension.
    constexpr T Size(const Int dimension) const noexcept {
        return max[dimension] - min[dimension];
    }

    /// Intersect with another bounds.
    void Intersect(const Bounds &other) {
        min = Point<T, N>{std::max(min[0], other.min[0]),
                          std::max(min[1], other.min[1])};
        max = Point<T, N>{std::min(max[0], other.max[0]),
                          std::min(max[1], other.max[1])};
    }

    /// Intersect with `Vector`.  Treat `Vector` as dimensions.
    template <arithmetic U> void Intersect(const Vector<U, N> &dim) {
        min = Point<T, N>{std::max(min[0], static_cast<T>(0)),
                          std::max(min[1], static_cast<T>(0))};
        max = Point<T, N>{std::min(max[0], static_cast<T>(dim[0])),
                          std::min(max[1], static_cast<T>(dim[1]))};
    }
};

export using Bounds2i = Bounds<Int, 2>;
export using Bounds2u = Bounds<UInt, 2>;

} // namespace rays
