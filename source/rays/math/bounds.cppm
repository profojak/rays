module;

#include <cstddef>

export module rays:bounds;

import :concepts;
import :point;

namespace rays {

export template <arithmetic T, std::size_t N>
    requires(N == 2 || N == 3)
struct Bounds {

    Point<T, N> min;
    Point<T, N> max;
};

} // namespace rays
