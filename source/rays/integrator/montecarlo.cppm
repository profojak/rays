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
                pixel = Pixel<T>{255.0f, 0.0f, 0.0f};
            }
        }
        film.PutTile(tile);

        // Simulate variable render cost so scheduler is observable.
        thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<std::chrono::milliseconds::rep> delay{
            1000, 2000};
        std::this_thread::sleep_for(std::chrono::milliseconds{delay(rng)});
    }
};

} // namespace rays
