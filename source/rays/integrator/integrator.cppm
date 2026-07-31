module;

#include <concepts>
#include <cstddef>
#include <optional>

export module rays:integrator;

import :film;
import :pixel;
import :scheduler;
import :thread;
import :tile;
import :type;

namespace rays {

/// Abstract base class for all integrators.
export template <std::floating_point T> class Integrator {
  public:
    /// Pure virtual function to render scene.
    virtual void Render(Film<T> &film, Scheduler<T> &scheduler,
                        ThreadPool &thread_pool) = 0;

    ~Integrator() = default;
};

/// Sampling integrator that samples radiance along rays.
export template <std::floating_point T>
class SamplingIntegrator : public Integrator<T> {
  public:
    SamplingIntegrator() = default;

    /// Render scene using ray sampling.
    void Render(Film<T> &film, Scheduler<T> &scheduler,
                ThreadPool &thread_pool) override {
        const auto num_threads = thread_pool.Size();
        for (std::size_t i = 0; i < num_threads; ++i) {
            thread_pool.Submit([this, &film, &scheduler] {
                while (std::optional<Tile<T>> tile = scheduler.NextTile()) {
                    RenderTile(*tile, film);
                }
            });
        }
    }

  protected:
    /// Render single `Tile` of `Film`.
    virtual void RenderTile(Tile<T> &tile, Film<T> &film) = 0;
};

} // namespace rays
