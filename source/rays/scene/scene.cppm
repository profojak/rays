module;

#include "state/options.h"

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

export module rays:scene;

import :camera;
import :light;
import :material;
import :mesh;
import :texture;
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

    /// Add `Texture` to scene.
    void AddTexture(const Texture &&texture) {
        textures_.push_back(std::move(texture));
    }

    /// Render scene to film.
    void Render(ThreadPool &thread_pool, Rays_Options &options) {
        camera_.Render(thread_pool, meshes_, lights_, materials_, options);
    }

    /// Preview scene with fast rendering.
    void Preview(ThreadPool &thread_pool, unsigned long long time_budget,
                 Rays_Options &options) {
        camera_.Preview(thread_pool, time_budget, meshes_, lights_, materials_,
                        options);
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
    /// Textures.
    std::vector<Texture> textures_;
};

} // namespace rays
