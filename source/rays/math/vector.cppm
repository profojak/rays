module;

#include "math.hpp" // IWYU pragma: keep

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

    /// Normalize vector.
    void Normalize() noexcept {
        const T norm = linalg::vector_two_norm(View());
        *this /= norm;
    }

    /// Cross product of two vectors.
    [[nodiscard]] static Vector Cross(const Vector &a,
                                      const Vector &b) noexcept {
        return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]};
    }

    /// Access element by index.
    template <typename Self>
    [[nodiscard]] constexpr auto operator[](this Self &&self,
                                            std::size_t i) noexcept
        -> decltype(auto) {
        return std::forward<Self>(self).data[i];
    }

    /// Unary plus.
    [[nodiscard]] constexpr auto operator+() const noexcept { return *this; }

    /// Unary minus.
    [[nodiscard]] constexpr auto operator-() const noexcept {
        Vector result{};
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
        Vector result{*this};
        for (std::size_t i = 0; i < N; ++i) {
            result.data[i] += scalar;
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
        Vector result{*this};
        linalg::scale(scalar, result.View());
        return result;
    }

    /// Divide vector by scalar.
    [[nodiscard]] constexpr auto operator/(T scalar) const noexcept {
        assert(scalar != 0);
        Vector result{*this};
        linalg::scale(1.0 / static_cast<Double>(scalar), result.View());
        return result;
    }

    /// Add vector.
    constexpr auto &operator+=(const Vector &other) noexcept {
        linalg::add(this->View(), other.View(), this->View());
        return *this;
    }

    /// Add scalar.
    constexpr auto &operator+=(T scalar) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] += scalar;
        }
        return *this;
    }

    /// Subtract vector.
    constexpr auto &operator-=(const Vector &other) noexcept {
        return *this += (-other);
    }

    /// Subtract scalar.
    constexpr auto &operator-=(T scalar) noexcept { return *this += (-scalar); }

    /// Multiply by scalar.
    constexpr auto &operator*=(T scalar) noexcept {
        linalg::scale(scalar, this->View());
        return *this;
    }

    /// Divide by scalar.
    constexpr auto &operator/=(T scalar) noexcept {
        assert(scalar != 0);
        linalg::scale(1.0 / static_cast<Double>(scalar), this->View());
        return *this;
    }
};

export using Vector2f = Vector<Float, 2>;
export using Vector3f = Vector<Float, 3>;
export using Vector2u = Vector<UInt, 2>;

} // namespace rays
