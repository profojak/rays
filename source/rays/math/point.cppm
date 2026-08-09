module;

#include "math.hpp" // IWYU pragma: keep

#include <array>
#include <type_traits>
#include <utility>

export module rays:point;

import :type;
import :vector;

namespace rays {

/// Point in space.
export template <arithmetic T, std::size_t N> struct Point {

    /// Point data.
    std::array<T, N> data{};

    /// Default constructor.
    Point() = default;

    /// Construct from scalar.
    template <arithmetic U> Point(const U scalar) {
        std::fill(this->data.begin(), this->data.end(), scalar);
    }

    /// Construct from scalars.
    template <arithmetic... Us>
        requires(sizeof...(Us) == N)
    Point(const Us... scalars) : data{static_cast<T>(scalars)...} {}

    /// Construct from `Point`.
    template <arithmetic U> Point(const Point<U, N> &other) {
        linalg::copy(this->View(), other.View(), this->View());
    }

    /// Construct from `Vector`.
    template <arithmetic U> Point(const Vector<U, N> &other) {
        linalg::copy(this->View(), other.View(), this->View());
    }

    /// Return `mdspan` view of point data.
    template <typename Self>
    [[nodiscard]] auto View(this Self &&self) noexcept {
        using element_type =
            std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
                               const T, T>;
        return std::mdspan<element_type, std::extents<std::size_t, N>>(
            std::forward<Self>(self).data.data());
    }

    /// Access element by index.
    template <typename Self>
    [[nodiscard]] constexpr auto operator[](this Self &&self,
                                            std::size_t i) noexcept
        -> decltype(auto) {
        return std::forward<Self>(self).data[i];
    }

    /// Add two points.
    [[nodiscard]] constexpr auto operator+(const Point &other) const noexcept {
        Point result{};
        linalg::add(this->View(), other.View(), result.View());
        return result;
    }

    /// Subtract vector from point.
    [[nodiscard]] constexpr auto
    operator-(const Vector<T, N> &other) const noexcept {
        return *this + (-other);
    }

    /// Multiply point by scalar.
    [[nodiscard]] constexpr auto operator*(T scalar) const noexcept {
        Point result{*this};
        linalg::scale(scalar, result.View());
        return result;
    }
};

export using Point2i = Point<Int, 2>;
export using Point2u = Point<UInt, 2>;
export using Point3f = Point<Float, 3>;

} // namespace rays
