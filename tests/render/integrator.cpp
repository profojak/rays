module;

#include "state/options.h"

#include <chrono>
#include <doctest/doctest.h>
#include <memory>
#include <thread>
#include <vector>

module rays;

import :film;
import :integrator;
import :light;
import :material;
import :matrix;
import :mesh;
import :ray;
import :scheduler;
import :thread;
import :tile;
import :type;
import :vector;

namespace {

/// Test integrator with artificial latency.
class SlowIntegrator : public rays::SamplingIntegrator<float> {
  public:
    using rays::SamplingIntegrator<float>::SamplingIntegrator;

  protected:
    void RenderTile(rays::Tile<float> &tile, rays::Film<float> &film) override {
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
        film.PutTile(tile);
    }

    [[nodiscard]] rays::Vector3f
    Sample(const rays::Ray3f &,
           const rays::Vector3f &) const noexcept override {
        return rays::Vector3f{1.0f};
    }
};

} // namespace

TEST_CASE("`SamplingIntegrator` detects render completion") {
    const rays::Vector2u resolution{16, 16};
    rays::Film<float> film{resolution};
    rays::SpiralScheduler<float> scheduler{resolution, 4};
    rays::ThreadPool thread_pool{1};
    SlowIntegrator integrator{resolution};

    const rays::Vector3f position{};
    const rays::Matrix3f rotation{};
    const std::vector<rays::Mesh> meshes;
    const std::vector<std::unique_ptr<rays::Light>> lights;
    const std::vector<rays::Material> materials;

    CHECK_FALSE(integrator.IsRendering());

    Rays_Options options;
    integrator.Render(position, rotation, meshes, lights, materials, film,
                      scheduler, thread_pool, options);

    CHECK(integrator.IsRendering());

    integrator.WaitForRender();

    CHECK_FALSE(integrator.IsRendering());
}

TEST_CASE("`SamplingIntegrator` is idle without render") {
    const rays::Vector2u resolution{16, 16};
    SlowIntegrator integrator{resolution};

    CHECK_FALSE(integrator.IsRendering());

    integrator.WaitForRender();
    CHECK_FALSE(integrator.IsRendering());
}
