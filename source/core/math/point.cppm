module;

#define MDSPAN_USE_PAREN_OPERATOR 1

#include <experimental/mdspan>

#include <array>
#include <concepts>
#include <type_traits>
#include <utility>

export module rays:point;

import :util;

namespace rays {

export template <arithmetic T, std::size_t N> struct Point {

  std::array<T, N> data{};

  template <typename Self> [[nodiscard]] auto View(this Self &&self) noexcept {
    using element_type =
        std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
                           const T, T>;
    return std::mdspan<element_type, std::extents<std::size_t, N>>(
        std::forward<Self>(self).data.data());
  }
};

export using Point2i = Point<int, 2>;

} // namespace rays
