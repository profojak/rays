export module rays:tile;

import :bounds;
import :point;

namespace rays {

/// Film tile.
export class Tile {
  public:
    Tile(const Bounds2u &bounds) : bounds_{bounds} {}

  private:
    /// Tile bounds in film.
    Bounds2u bounds_;
};

} // namespace rays
