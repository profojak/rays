module;

#include <concepts>
#include <vector>

export module rays:film;

import :vector;

export namespace rays {

/// 2D raster of accumulated color samples.
template <std::floating_point T> class Film {
  public:
    Film() noexcept = default;
    Film(std::size_t width, std::size_t height)
        : width_{width}, height_{height}, image_(width * height) {}

  private:
    std::size_t width_{0};
    std::size_t height_{0};
    std::vector<Vector3f> image_{};
};

} // namespace rays
