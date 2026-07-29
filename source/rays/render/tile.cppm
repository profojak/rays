export module rays:tile;

import :bounds;
import :point;

namespace rays {

/// Film tile.
export class Tile {
  public:
    Tile(const Bounds2u &bounds) : bounds_{bounds} {}

    /// Return tile bounds.
    [[nodiscard]] const Bounds2u &Bounds() const noexcept { return bounds_; }

  private:
    /// Tile bounds in film.
    Bounds2u bounds_;
};

} // namespace rays
