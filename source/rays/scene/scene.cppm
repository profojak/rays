module;

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

export module rays:scene;

import :camera;
import :light;
import :material;
import :mesh;
import :thread;
import :type;

namespace rays {

/// Scene.
export class Scene {

  public:
    /// Return reference to `Camera`.
    template <typename Self> [[nodiscard]] auto &GetCamera(this Self &&self) {
        return std::forward<Self>(self).camera_;
    }

    /// Add `Mesh` to scene.
    void AddMesh(const Mesh &&mesh) { meshes_.push_back(std::move(mesh)); }

    /// Add `Light` to scene.
    template <std::derived_from<Light> L> void AddLight(const L &light) {
        lights_.push_back(std::make_unique<L>(light));
    }

    /// Add `Material` to scene.
    void AddMaterial(const Material &&material) {
        materials_.push_back(std::move(material));
    }

    /// Render scene to film.
    void Render(ThreadPool &thread_pool) {
        camera_.Render(thread_pool, meshes_, lights_, materials_);
    }

    /// Preview scene with fast rendering.
    void Preview(ThreadPool &thread_pool, unsigned long long time_budget) {
        camera_.Preview(thread_pool, time_budget, meshes_, lights_, materials_);
    }

  private:
    /// Camera.
    Camera<Float> camera_;
    /// Meshes.
    std::vector<Mesh> meshes_;
    /// Lights.
    std::vector<std::unique_ptr<Light>> lights_;
    /// Materials.
    std::vector<Material> materials_;
};

} // namespace rays
