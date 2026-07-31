module;

#include <memory>

export module rays:state;

import :camera;

namespace rays {

/// Internal state of renderer.
export struct State {
    /// Camera used to render scene.
    inline static std::unique_ptr<Camera<float>> camera;
};

} // namespace rays
