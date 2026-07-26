module;

#include <concepts>
#include <utility>
#include <vector>

export module rays:film;

import :pixel;

export namespace rays {

/// 2D raster of accumulated `Pixel` samples.
template <std::floating_point T> class Film {
public:
  Film() noexcept = default;
  Film(std::size_t width, std::size_t height)
      : width_{width}, height_{height}, pixels_(width * height) {}

  /// Access underlying flat row-major `Pixel` storage.
  template <typename Self>
  [[nodiscard]] constexpr auto data(this Self &&self) noexcept -> auto & {
    return std::forward<Self>(self).pixels_;
  }

private:
  std::size_t width_{0};
  std::size_t height_{0};
  std::vector<Pixel<T>> pixels_{};
};

} // namespace rays
