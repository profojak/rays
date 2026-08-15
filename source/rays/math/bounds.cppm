module;

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>

export module rays:bounds;

import :point;
import :ray;
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

    Bounds()
        : min{std::numeric_limits<T>::infinity()},
          max{-std::numeric_limits<T>::infinity()} {}

    /// Construct from size.
    template <arithmetic U> Bounds(const U size) : min{Point<T, N>{0}} {
        max = Point<T, N>{size};
    }

    /// Construct from minimum point and size.
    template <arithmetic U, arithmetic V>
    Bounds(const Point<U, N> &min, const V size) : min{min} {
        max = min + Point<V, N>{size};
    }

    /// Construct from scalar dimensions.
    template <arithmetic... Us>
        requires(sizeof...(Us) == N)
    Bounds(const Us... size) : min{Point<T, N>{0}} {
        max = Point<T, N>{size...};
    }

    /// Construct from minimum point and scalar dimensions.
    template <arithmetic... Us>
        requires(sizeof...(Us) == N)
    Bounds(const Point<T, N> &min, const Us... size) : min{min} {
        max = min + Point<T, N>{size...};
    }

    /// Return size along given dimension.
    constexpr T Size(const Int dimension) const noexcept {
        return max[dimension] - min[dimension];
    }

    /// Return area of 2D bounds.
    constexpr T Area2D() const noexcept
        requires(N == 2)
    {
        return Size(0) * Size(1);
    }

    /// Return surface area of 3D bounds.
    constexpr T Area3D() const noexcept
        requires(N == 3)
    {
        const T dx = Size(0);
        const T dy = Size(1);
        const T dz = Size(2);
        return T{2} * (dx * dy + dy * dz + dz * dx);
    }

    /// Return center point.
    [[nodiscard]] constexpr Point<T, N> Center() const noexcept {
        Point<T, N> result;
        for (std::size_t i = 0; i < N; ++i)
            result[i] = (min[i] + max[i]) * T{0.5};
        return result;
    }

    /// Return dimension index of greatest extent.
    [[nodiscard]] constexpr UInt MajorAxis() const noexcept {
        UInt largest = 0;
        T best = Size(0);
        for (std::size_t i = 1; i < N; ++i) {
            const T extent = Size(static_cast<Int>(i));
            if (extent > best) {
                best = extent;
                largest = static_cast<UInt>(i);
            }
        }
        return largest;
    }

    /// Return whether box encloses any space.
    [[nodiscard]] constexpr bool IsValid() const noexcept {
        for (std::size_t i = 0; i < N; ++i)
            if (max[i] < min[i])
                return false;
        return true;
    }

    /// Expand to contain anything with component-wise access, such as `Point`
    /// or `Vector`.  Exclude `Bounds` so dedicated overload below is preferred.
    template <typename U>
        requires requires(const U &x, std::size_t i) {
            { x[i] } -> std::convertible_to<T>;
        } && (!std::same_as<std::remove_cvref_t<U>, Bounds>)
    void Expand(const U &p) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            min[i] = std::min(min[i], static_cast<T>(p[i]));
            max[i] = std::max(max[i], static_cast<T>(p[i]));
        }
    }

    /// Expand to contain another bounds.
    void Expand(const Bounds &other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            min[i] = std::min(min[i], other.min[i]);
            max[i] = std::max(max[i], other.max[i]);
        }
    }

    /// Intersect with another bounds.
    void Intersect(const Bounds &other) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            min[i] = std::max(min[i], other.min[i]);
            max[i] = std::min(max[i], other.max[i]);
        }
    }

    /// Intersect with `Vector`.  Treat `Vector` as dimensions.
    template <arithmetic U> void Intersect(const Vector<U, N> &dim) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            min[i] = std::max(min[i], static_cast<T>(0));
            max[i] = std::min(max[i], static_cast<T>(dim[i]));
        }
    }

    /// Intersect with `Ray`.
    template <std::floating_point U>
        requires(N == 3)
    [[nodiscard]] bool Intersect(const Ray<U> &ray, U &near_t,
                                 U &far_t) const noexcept {
        near_t = -std::numeric_limits<U>::infinity();
        far_t = std::numeric_limits<U>::infinity();

        for (std::size_t i = 0; i < 3; ++i) {
            const U origin = static_cast<U>(ray.origin[i]);
            const U min_val = static_cast<U>(min[i]);
            const U max_val = static_cast<U>(max[i]);
            const U direction = static_cast<U>(ray.direction[i]);

            if (direction == U{0}) {
                if (origin < min_val || origin > max_val)
                    return false;
            } else {
                const U inv_direction = U{1} / direction;
                U t1 = (min_val - origin) * inv_direction;
                U t2 = (max_val - origin) * inv_direction;
                if (t1 > t2)
                    std::swap(t1, t2);
                near_t = std::max(near_t, t1);
                far_t = std::min(far_t, t2);
                if (near_t > far_t)
                    return false;
            }
        }

        return true;
    }

    /// Intersect with `Ray`.
    template <std::floating_point U>
        requires(N == 3)
    [[nodiscard]] bool Intersect(const Ray<U> &ray) const noexcept {
        U near_t, far_t;
        return Intersect(ray, near_t, far_t);
    }
};

export using Bounds2i = Bounds<Int, 2>;
export using Bounds2u = Bounds<UInt, 2>;
export using Bounds3f = Bounds<Float, 3>;

} // namespace rays
