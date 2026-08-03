module;

export module rays:scene;

import :camera;
import :type;

namespace rays {

/// Scene.
export class Scene {

  private:
    /// Camera.
    Camera<Float> camera_;
};

} // namespace rays
