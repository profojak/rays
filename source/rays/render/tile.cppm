module;

#include <concepts>
#include <vector>

export module rays:tile;

import :bounds;
import :pixel;
import :point;

namespace rays {

/// Film tile.
export template <std::floating_point T> class Tile {
  public:
    Tile(const Bounds2u &bounds) : bounds_{bounds}, pixels_{bounds.Area()} {}

    /// Return reference to pixel at absolute film coordinates.
    template <typename Self>
    [[nodiscard]] auto &PixelAt(this Self &&self, UInt x, UInt y) {
        return self.pixels_[(y - self.bounds_.min[1]) * self.bounds_.Size(0) +
                            (x - self.bounds_.min[0])];
    }

    /// Return tile bounds.
    [[nodiscard]] const Bounds2u &Bounds() const noexcept { return bounds_; }

  private:
    /// Tile bounds in film.
    Bounds2u bounds_;
    /// Tile pixels.
    std::vector<Pixel<T>> pixels_;
};

} // namespace rays
