module;

#include <concepts>
#include <vector>

export module rays:camera;

import :film;
import :matrix;
import :mesh;
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
        : film_{film_size}, monte_carlo_{film_size}, scheduler_{film_size, 64} {
    }

    /// Return reference to `Film`.
    template <typename Self> [[nodiscard]] auto &GetFilm(this Self &&self) {
        return std::forward<Self>(self).film_;
    }

    /// Set film size.
    void SetFilmSize(const Vector2u &film_size) {
        film_.SetResolution(film_size);
        monte_carlo_.SetFilmSize(film_size);
        scheduler_.SetFilmSize(film_size);
    }

    /// Set camera position.
    void SetPosition(const Vector3f &position) { position_ = position; }

    /// Set camera rotation.
    void SetRotation(const Matrix3f &rotation) { rotation_ = rotation; }

    /// Kick off render only if no render is currently in progress.
    void Render(ThreadPool &thread_pool, const std::vector<Mesh> &meshes) {
        if (!thread_pool.IsIdle()) {
            return;
        }
        scheduler_.Reset();
        monte_carlo_.Render(position_, rotation_, meshes, film_, scheduler_,
                            thread_pool);
    }

  private:
    /// Camera position.
    Vector3f position_{};
    /// Camera rotation.
    Matrix3f rotation_{};

    /// Camera film.
    Film<T> film_{Vector2u{1280, 720}};
    /// Monte Carlo integrator.
    MonteCarloIntegrator<T> monte_carlo_{Vector2u{1280, 720}};
    /// Scheduler for tile sampling.
    SpiralScheduler<T> scheduler_{Vector2u{1280, 720}, 64};
};

} // namespace rays
