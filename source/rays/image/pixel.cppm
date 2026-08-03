module;

#include <array>
#include <concepts>
#include <utility>

export module rays:pixel;

import :vector;

export namespace rays {

/// Pixel.
template <std::floating_point T> struct Pixel {

    /// RGB components.
    std::array<T, 3> rgb{0.0};

    constexpr Pixel() noexcept = default;
    constexpr Pixel(T r, T g, T b) noexcept : rgb{r, g, b} {}
    constexpr Pixel(Vector3f color) noexcept
        : rgb{color[0], color[1], color[2]} {}

    /// Access red component.
    template <typename Self>
    [[nodiscard]] constexpr auto R(this Self &&self) noexcept -> auto & {
        return std::forward<Self>(self).rgb[0];
    }

    /// Access green component.
    template <typename Self>
    [[nodiscard]] constexpr auto G(this Self &&self) noexcept -> auto & {
        return std::forward<Self>(self).rgb[1];
    }

    /// Access blue component.
    template <typename Self>
    [[nodiscard]] constexpr auto B(this Self &&self) noexcept -> auto & {
        return std::forward<Self>(self).rgb[2];
    }
};

} // namespace rays
