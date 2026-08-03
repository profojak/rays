module;

#include <utility>

export module rays:scene;

import :camera;
import :type;

namespace rays {

/// Scene.
export class Scene {

  public:
    /// Return reference to `Camera`.
    template <typename Self> [[nodiscard]] auto &GetCamera(this Self &&self) {
        return std::forward<Self>(self).camera_;
    }

  private:
    /// Camera.
    Camera<Float> camera_;
};

} // namespace rays
