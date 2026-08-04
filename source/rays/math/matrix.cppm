module;

#include "math.hpp" // IWYU pragma: keep

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

export module rays:matrix;

import :type;
import :vector;

namespace rays {

/// Linear algebra matrix in row-major order.
export template <arithmetic T, std::size_t R, std::size_t C> struct Matrix {

    /// Matrix data.
    std::array<T, R * C> data{};

    /// Default constructor.
    Matrix() = default;

    /// Construct from scalar.
    template <arithmetic U> Matrix(const U scalar) {
        std::fill(this->data.begin(), this->data.end(), scalar);
    }

    /// Construct from scalars in row-major order.
    template <arithmetic... Us>
        requires(sizeof...(Us) == R * C && sizeof...(Us) != 1)
    Matrix(const Us... scalars) : data{static_cast<T>(scalars)...} {}

    /// Construct from `array`.
    template <std::size_t N>
    Matrix(const std::array<T, N> &other)
        requires(N == R * C)
    {
        std::copy(other.begin(), other.end(), this->data.begin());
    }

    /// Construct from `Matrix`.
    template <arithmetic U> Matrix(const Matrix<U, R, C> &other) {
        linalg::copy(other.View(), this->View());
    }

    /// Return `mdspan` view of matrix data.
    template <typename Self>
    [[nodiscard]] auto View(this Self &&self) noexcept {
        using element_type =
            std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>,
                               const T, T>;
        return std::mdspan<element_type, std::extents<std::size_t, R, C>>(
            std::forward<Self>(self).data.data());
    }

    /// Access element by index.
    template <typename Self>
    [[nodiscard]] constexpr auto operator[](this Self &&self,
                                            std::size_t i) noexcept
        -> decltype(auto) {
        return std::forward<Self>(self).data[i];
    }

    /// Access element by row and column.
    template <typename Self>
    [[nodiscard]] constexpr auto operator[](this Self &&self, std::size_t row,
                                            std::size_t col) noexcept
        -> decltype(auto) {
        return std::forward<Self>(self).data[row * C + col];
    }

    /// Unary plus.
    [[nodiscard]] constexpr auto operator+() const noexcept { return *this; }

    /// Unary minus.
    [[nodiscard]] constexpr auto operator-() const noexcept {
        Matrix result{*this};
        linalg::scale(T{-1}, result.View());
        return result;
    }

    /// Add two matrices.
    [[nodiscard]] constexpr auto operator+(const Matrix &other) const noexcept {
        Matrix result{};
        linalg::add(this->View(), other.View(), result.View());
        return result;
    }

    /// Subtract two matrices.
    [[nodiscard]] constexpr auto operator-(const Matrix &other) const noexcept {
        return *this + (-other);
    }

    /// Multiply matrix by scalar.
    [[nodiscard]] constexpr auto operator*(T scalar) const noexcept {
        Matrix result{*this};
        linalg::scale(scalar, result.View());
        return result;
    }

    /// Divide matrix by scalar.
    [[nodiscard]] constexpr auto operator/(T scalar) const noexcept {
        assert(scalar != 0);
        Matrix result{*this};
        linalg::scale(1.0 / static_cast<Double>(scalar), result.View());
        return result;
    }

    /// Multiply matrix by vector.
    [[nodiscard]] constexpr auto
    operator*(const Vector<T, C> &vector) const noexcept {
        Vector<T, R> result{};
        linalg::matrix_vector_product(this->View(), vector.View(),
                                      result.View());
        return result;
    }

    /// Multiply matrix by matrix.
    template <arithmetic U, std::size_t K>
    [[nodiscard]] constexpr auto
    operator*(const Matrix<U, C, K> &other) const noexcept {
        Matrix<T, R, K> result{};
        linalg::matrix_product(this->View(), other.View(), result.View());
        return result;
    }

    /// Add matrix.
    constexpr auto &operator+=(const Matrix &other) noexcept {
        linalg::add(this->View(), other.View(), this->View());
        return *this;
    }

    /// Subtract matrix.
    constexpr auto &operator-=(const Matrix &other) noexcept {
        return *this += (-other);
    }

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

    /// Return transpose of matrix.
    [[nodiscard]] constexpr auto Transposed() const noexcept {
        Matrix<T, C, R> result{};
        linalg::copy(linalg::transposed(this->View()), result.View());
        return result;
    }

    /// Return identity matrix.
    [[nodiscard]] static constexpr Matrix Identity() noexcept
        requires(R == C)
    {
        Matrix result{};
        for (std::size_t i = 0; i < R; ++i) {
            result[i, i] = T{1};
        }
        return result;
    }

    /// Compare for equality.
    [[nodiscard]] constexpr bool
    operator==(const Matrix &other) const noexcept {
        return this->data == other.data;
    }
};

export using Matrix2f = Matrix<Float, 2, 2>;
export using Matrix3f = Matrix<Float, 3, 3>;
export using Matrix4f = Matrix<Float, 4, 4>;

} // namespace rays
