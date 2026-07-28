module;

#include <cstddef>

export module rays:bounds;

import :concepts;
import :point;

namespace rays {

/// Bounds.
export template <arithmetic T, std::size_t N>
    requires(N == 2 || N == 3)
struct Bounds {

    /// Minimum point.
    Point<T, N> min;
    /// Maximum point.
    Point<T, N> max;
};

export using Bounds2i = Bounds<int, 2>;

} // namespace rays
