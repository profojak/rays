module;

#include <concepts>

export module rays:camera;

import :film;
import :montecarlo;
import :scheduler;
import :thread;
import :vector;

namespace rays {

/// Camera that renders scene to film.
export template <std::floating_point T> class Camera {
  public:
    Camera() = default;

    Camera(const Vector2u &film_size)
        : film_{film_size}, scheduler_{film_size, 64} {}

    /// Return reference to `Film`.
    template <typename Self> [[nodiscard]] auto &GetFilm(this Self &&self) {
        return std::forward<Self>(self).film_;
    }

    /// Kick off render only if no render is currently in progress.
    void Render(ThreadPool &thread_pool) {
        if (!thread_pool.IsIdle()) {
            return;
        }
        scheduler_.Reset();
        monte_carlo_.Render(film_, scheduler_, thread_pool);
    }

  private:
    /// Camera film.
    Film<T> film_{Vector2u{1280, 720}};
    /// Monte Carlo integrator.
    MonteCarloIntegrator<T> monte_carlo_{};
    /// Scheduler for tile sampling.
    SpiralScheduler<T> scheduler_{Vector2u{1280, 720}, 64};
};

} // namespace rays
