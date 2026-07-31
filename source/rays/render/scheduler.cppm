module;

#include <cassert>
#include <concepts>
#include <mutex>
#include <optional>
#include <utility>

export module rays:scheduler;

import :point;
import :tile;
import :vector;

namespace rays {

/// Abstract base class for tile scheduler.
export template <std::floating_point T> class Scheduler {

  public:
    virtual ~Scheduler() = default;

    /// Pure virtual method to get next tile.
    virtual std::optional<Tile<T>> NextTile() = 0;

  protected:
    Scheduler() = default;
};

/// Scheduler that traverses film in spiral pattern.
export template <std::floating_point T>
class SpiralScheduler : public Scheduler<T> {

  public:
    /// Initialize with film resolution.
    SpiralScheduler(const Vector2u &film_resolution, const UInt block_size)
        : film_resolution_{film_resolution}, block_size_{block_size} {
        block_grid_size_ = (film_resolution + block_size_ - 1) / block_size_;
        block_count_ = block_grid_size_[0] * block_grid_size_[1];
        Reset();
    }

    ~SpiralScheduler() override = default;

    /// Reset scheduler to initial state.
    void Reset() {
        block_position_ = Point2u{(block_grid_size_ - 1u) / 2u};
        block_counter_ = 0;
        direction_ = Direction::Right;
        spiral_steps_ = 1;
        spiral_size_ = 1;
    }

    /// Get next tile in spiral pattern.
    std::optional<Tile<T>> NextTile() override {
        std::lock_guard<std::mutex> lock(mutex_);

        while (block_counter_ != block_count_) {
            const Point2u pos = block_position_;
            const bool in_grid =
                pos[0] < block_grid_size_[0] && pos[1] < block_grid_size_[1];

            switch (direction_) {
            case Direction::Right:
                ++block_position_[0];
                break;
            case Direction::Down:
                ++block_position_[1];
                break;
            case Direction::Left:
                --block_position_[0];
                break;
            case Direction::Up:
                --block_position_[1];
                break;
            }
            if (--spiral_steps_ == 0) {
                direction_ =
                    Direction{(std::to_underlying(direction_) + 1) % 4};
                if (direction_ == Direction::Right ||
                    direction_ == Direction::Left) {
                    ++spiral_size_;
                }
                spiral_steps_ = spiral_size_;
            }

            if (!in_grid) {
                continue;
            }

            Bounds2u bounds{pos * block_size_, block_size_};
            bounds.Intersect(film_resolution_);

            assert(bounds.Size(0) > 0 && bounds.Size(1) > 0);

            ++block_counter_;
            return Tile<T>{bounds};
        }
        return std::nullopt;
    }

  private:
    /// Direction of spiral.
    enum class Direction { Right, Down, Left, Up };

    /// Film resolution.
    Vector2u film_resolution_;
    /// Block grid size.
    Vector2u block_grid_size_;
    /// Current block position.
    Point2u block_position_;
    /// Block size.
    UInt block_size_;
    /// Block count.
    UInt block_count_;
    /// Block counter.
    UInt block_counter_;
    /// Current direction of spiral.
    Direction direction_;
    /// Steps left before changing direction.
    UInt spiral_steps_;
    /// Current spiral size in blocks.
    UInt spiral_size_;

    /// Mutex for thread-safe tile generation.
    std::mutex mutex_;
};

} // namespace rays
