module;

#include <chrono>
#include <concepts>
#include <random>
#include <thread>

export module rays:montecarlo;

import :integrator;

namespace rays {

/// Monte Carlo integrator that samples radiance along rays.
export template <std::floating_point T>
class MonteCarloIntegrator : public SamplingIntegrator<T> {
  public:
    MonteCarloIntegrator() = default;

  protected:
    /// Render single `Tile` of `Film`.
    void RenderTile(Tile<T> &tile, Film<T> &film) override {
        const auto &bounds = tile.Bounds();
        for (UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
            for (UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
                auto &pixel = tile.PixelAt(x, y);
                pixel = film.GetBackground();
            }
        }
        film.PutTile(tile);
    }
};

} // namespace rays
