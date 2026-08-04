module;

#include <concepts>
#include <cstddef>
#include <optional>
#include <vector>

export module rays:integrator;

import :film;
import :mesh;
import :matrix;
import :pixel;
import :scheduler;
import :thread;
import :tile;
import :type;
import :vector;

namespace rays {

/// Abstract base class for all integrators.
export template <std::floating_point T> class Integrator {
  public:
    /// Pure virtual function to render scene.
    virtual void Render(const Vector3f &camera_position,
                        const Matrix3f &camera_rotation,
                        const std::vector<Mesh> &meshes, Film<T> &film,
                        Scheduler<T> &scheduler, ThreadPool &thread_pool) = 0;

    ~Integrator() = default;
};

/// Sampling integrator that samples radiance along rays.
export template <std::floating_point T>
class SamplingIntegrator : public Integrator<T> {
  public:
    SamplingIntegrator() = default;

    /// Render scene using ray sampling.
    void Render(const Vector3f &camera_position,
                const Matrix3f &camera_rotation,
                const std::vector<Mesh> &meshes, Film<T> &film,
                Scheduler<T> &scheduler, ThreadPool &thread_pool) override {
        position_ = camera_position;
        rotation_ = camera_rotation;
        meshes_ = &meshes;
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

    /// Camera position during render.
    Vector3f position_{};
    /// Camera rotation during render.
    Matrix3f rotation_{};
    /// Scene meshes during render.
    const std::vector<Mesh> *meshes_{};
};

} // namespace rays
