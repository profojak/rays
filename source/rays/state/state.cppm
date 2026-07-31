module;

#include <memory>

export module rays:state;

import :camera;
import :thread;

namespace rays {

/// Internal state of renderer.
export struct State {
    /// Camera used to render scene.
    inline static std::unique_ptr<Camera<float>> camera;
    /// Pool of worker threads used by integrators for tiled rendering.
    inline static ThreadPool thread_pool;
};

} // namespace rays
