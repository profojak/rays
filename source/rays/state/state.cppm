module;

#include <memory>

export module rays:state;

import :scene;
import :thread;

namespace rays {

/// Internal state of renderer.
export struct State {
    /// Scene.
    inline static std::unique_ptr<Scene> scene;
    /// Pool of worker threads used by integrators for tiled rendering.
    inline static ThreadPool thread_pool;
};

} // namespace rays
