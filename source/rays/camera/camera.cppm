module;

#include <chrono>
#include <concepts>
#include <vector>

export module rays:camera;

import :film;
import :matrix;
import :mesh;
import :montecarlo;
import :preview;
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
        preview_.SetFilmSize(film_size);
        scheduler_.SetFilmSize(film_size);
    }

    /// Set camera position.
    void SetPosition(const Vector3f &position) { position_ = position; }

    /// Get camera position.
    [[nodiscard]] Vector3f GetPosition() const { return position_; }

    /// Set camera rotation.
    void SetRotation(const Matrix3f &rotation) { rotation_ = rotation; }

    /// Get camera rotation.
    [[nodiscard]] Matrix3f GetRotation() const { return rotation_; }

    /// Kick off render only if no render is currently in progress.
    void Render(ThreadPool &thread_pool, const std::vector<Mesh> &meshes) {
        if (!thread_pool.IsIdle()) {
            return;
        }
        scheduler_.Reset();
        monte_carlo_.Render(position_, rotation_, meshes, film_, scheduler_,
                            thread_pool);
    }

    /// Preview scene with fast rendering.
    void Preview(ThreadPool &thread_pool, unsigned long long time_budget,
                 const std::vector<Mesh> &meshes) {
        if (!thread_pool.IsIdle()) {
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();
        auto end = start;
        preview_.Reset();

        do {
            preview_.Pass();
            scheduler_.Reset();
            preview_.Render(position_, rotation_, meshes, film_, scheduler_,
                            thread_pool);
            // Uncomment to simulate longer preview passes.
            // std::this_thread::sleep_for(std::chrono::milliseconds(8));
            thread_pool.WaitIdle();

            end = std::chrono::high_resolution_clock::now();
        } while (
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count() < time_budget);
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
    /// Preview integrator.
    PreviewIntegrator<T> preview_{Vector2u{1280, 720}};
    /// Scheduler for tile sampling.
    SpiralScheduler<T> scheduler_{Vector2u{1280, 720}, 64};
};

} // namespace rays
