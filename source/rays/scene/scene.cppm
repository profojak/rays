module;

#include <utility>
#include <vector>

export module rays:scene;

import :camera;
import :mesh;
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

  private:
    /// Camera.
    Camera<Float> camera_;
    /// Meshes.
    std::vector<Mesh> meshes_;
};

} // namespace rays
