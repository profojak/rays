module;

#include "state/options.h"

#include <atomic>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

export module rays:integrator;

import :bvh;
import :film;
import :light;
import :material;
import :mesh;
import :matrix;
import :pixel;
import :scheduler;
import :texture;
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
                        const std::vector<Mesh> &meshes,
                        const std::vector<std::unique_ptr<Light>> &lights,
                        const std::vector<Material> &materials,
                        const std::vector<Texture> &textures, Film<T> &film,
                        Scheduler<T> &scheduler, ThreadPool &thread_pool,
                        Rays_Options &options) = 0;

    ~Integrator() = default;
};

/// Sampling integrator that samples radiance along rays.
export template <std::floating_point T>
class SamplingIntegrator : public Integrator<T> {
  public:
    SamplingIntegrator(const Vector2u &film_size) { SetFilmSize(film_size); }

    /// Render scene using ray sampling.
    void Render(const Vector3f &camera_position,
                const Matrix3f &camera_rotation,
                const std::vector<Mesh> &meshes,
                const std::vector<std::unique_ptr<Light>> &lights,
                const std::vector<Material> &materials,
                const std::vector<Texture> &textures, Film<T> &film,
                Scheduler<T> &scheduler, ThreadPool &thread_pool,
                Rays_Options &options) override {
        position_ = camera_position;
        rotation_ = camera_rotation;
        meshes_ = &meshes;
        lights_ = &lights;
        materials_ = &materials;
        textures_ = &textures;
        options_ = &options;

        bvh_.Build(meshes, materials);

        const auto num_threads = thread_pool.Size();
        tasks_remaining_.store(num_threads, std::memory_order_relaxed);
        for (std::size_t i = 0; i < num_threads; ++i) {
            thread_pool.Submit([this, &film, &scheduler] {
                while (std::optional<Tile<T>> tile = scheduler.NextTile()) {
                    RenderTile(*tile, film);
                }
                if (tasks_remaining_.fetch_sub(1, std::memory_order_acq_rel) ==
                    1) {
                    tasks_remaining_.notify_all();
                }
            });
        }
    }

    /// Check if render is currently in progress.
    [[nodiscard]] bool IsRendering() const noexcept {
        return tasks_remaining_.load(std::memory_order_acquire) != 0;
    }

    /// Block until in-progress render has finished.
    void WaitForRender() {
        auto remaining = tasks_remaining_.load(std::memory_order_acquire);
        while (remaining != 0) {
            tasks_remaining_.wait(remaining, std::memory_order_acquire);
            remaining = tasks_remaining_.load(std::memory_order_acquire);
        }
    }

    /// Set film size.
    void SetFilmSize(const Vector2u &film_size) {
        film_size_ = film_size;
        aspect_ = static_cast<Float>(film_size_[0]) /
                  static_cast<Float>(film_size_[1]);
        inverse_uv_ =
            Vector2f{1.0f / static_cast<Float>(film_size_[0]) * Float{2},
                     1.0f / static_cast<Float>(film_size_[1]) * Float{2}};
    }

  protected:
    /// Render single `Tile` of `Film`.
    virtual void RenderTile(Tile<T> &tile, Film<T> &film) = 0;

    /// Sample ray and return color.
    [[nodiscard]] virtual Vector3f
    Sample(const Ray3f &ray, const Vector3f &background) const noexcept = 0;

    /// Camera position during render.
    Vector3f position_{};
    /// Camera rotation during render.
    Matrix3f rotation_{};

    /// Film size.
    Vector2u film_size_{};
    /// Inverse UV coordinates.
    Vector2f inverse_uv_{};
    /// Aspect ratio.
    Float aspect_{};

    /// Render options.
    Rays_Options *options_{};

    /// Bounding volume hierarchy used to accelerate intersections.
    BVH bvh_;

    /// Scene meshes during render.
    const std::vector<Mesh> *meshes_{};
    /// Scene lights during render.
    const std::vector<std::unique_ptr<Light>> *lights_{};
    /// Scene materials during render.
    const std::vector<Material> *materials_{};
    /// Scene textures during render.
    const std::vector<Texture> *textures_{};
    /// Number of render tasks not yet finished.
    std::atomic<std::size_t> tasks_remaining_{0};
};

} // namespace rays
