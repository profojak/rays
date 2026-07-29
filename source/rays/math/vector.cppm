module;

#include "math.hpp"

#include <array>
#include <cassert>
#include <type_traits>
#include <utility>

export module rays:vector;

import :type;

namespace rays {

/// Linear algebra vector.
export template <arithmetic T, std::size_t N> struct Vector {

    /// Vector data.
    std::array<T, N> data{};

    /// Return `mdspan` view of vector data.
    template <typename Self>
    [[nodiscard]] auto View(this Self &&self) noexcept {
        using element_type =
            std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
                               const T, T>;
        return std::mdspan<element_type, std::extents<std::size_t, N>>(
            std::forward<Self>(self).data.data());
    }

    /// Unary plus.
    [[nodiscard]] constexpr auto operator+() const noexcept { return *this; }

    /// Unary minus.
    [[nodiscard]] constexpr auto operator-() const noexcept {
        Vector<T, N> result{};
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = -data[i];
        }
        return result;
    }

    /// Add two vectors.
    [[nodiscard]] constexpr auto operator+(const Vector &other) const noexcept {
        Vector result{};
        linalg::add(this->View(), other.View(), result.View());
        return result;
    }

    /// Add scalar to vector.
    [[nodiscard]] constexpr auto operator+(T scalar) const noexcept {
        Vector result{};
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] = data[i] + scalar;
        }
        return result;
    }

    /// Subtract two vectors.
    [[nodiscard]] constexpr auto operator-(const Vector &other) const noexcept {
        return *this + (-other);
    }

    /// Subtract scalar from vector.
    [[nodiscard]] constexpr auto operator-(T scalar) const noexcept {
        return *this + (-scalar);
    }

    /// Multiply vector by scalar.
    [[nodiscard]] constexpr auto operator*(T scalar) const noexcept {
        Vector result{};
        linalg::scale(this->View(), scalar, result.View());
        return result;
    }

    /// Divide vector by scalar.
    [[nodiscard]] constexpr auto operator/(T scalar) const noexcept {
        assert(scalar != 0);

        Vector result{};
        linalg::scale(this->View(), static_cast<T>(1) / scalar, result.View());
        return result;
    }

    /// Add vector.
    [[nodiscard]] constexpr auto &operator+=(const Vector &other) noexcept {
        linalg::add(this->View(), other.View(), this->View());
        return *this;
    }

    /// Add scalar.
    [[nodiscard]] constexpr auto &operator+=(T scalar) noexcept {
        linalg::add(this->View(), scalar, this->View());
        return *this;
    }

    /// Subtract vector.
    [[nodiscard]] constexpr auto &operator-=(const Vector &other) noexcept {
        return *this += (-other);
    }

    /// Subtract scalar.
    [[nodiscard]] constexpr auto &operator-=(T scalar) noexcept {
        return *this += (-scalar);
    }

    /// Multiply by scalar.
    [[nodiscard]] constexpr auto &operator*=(T scalar) noexcept {
        linalg::scale(this->View(), scalar, this->View());
        return *this;
    }

    /// Divide by scalar.
    [[nodiscard]] constexpr auto &operator/=(T scalar) noexcept {
        return *this *= (static_cast<T>(1) / scalar);
    }
};

export using Vector3f = Vector<Float, 3>;
export using Vector2u = Vector<UInt, 2>;

} // namespace rays
