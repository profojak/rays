export module rays:tile;

import :bounds;
import :point;

namespace rays {

/// Film tile.
export class Tile {

  private:
    /// Tile position in film.
    Point2i position_;
    /// Tile bounds in film.
    Bounds2i bounds_;
};

} // namespace rays
