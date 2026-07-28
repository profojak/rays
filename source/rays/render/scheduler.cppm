module;

#include <optional>

export module rays:scheduler;

import :bounds;
import :tile;

namespace rays {

/// Abstract base class for tile scheduler.
export class Scheduler {

  public:
    virtual ~Scheduler() = default;

    /// Pure virtual method to get next tile.
    virtual std::optional<Tile> NextTile() = 0;

  protected:
    Scheduler() = default;
};

/// Scheduler that traverses film in spiral pattern.
export class SpiralScheduler : public Scheduler {

  public:
    /// Initialize with film resolution.
    SpiralScheduler(const Bounds2i &film_resolution);

    ~SpiralScheduler() override = default;

    /// Get next tile in spiral pattern.
    std::optional<Tile> NextTile() override;

  private:
    /// Film resolution.
    Bounds2i film_resolution_;
};

} // namespace rays
