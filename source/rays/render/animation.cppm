module;

#include <vector>

export module rays:animation;

import :matrix;
import :type;
import :vector;

namespace rays {

/// Keyframe with value at given time.
export template <typename T> struct Keyframe {
    /// Time of keyframe in seconds.
    Float time;
    /// Value at keyframe.
    T value;
};

/// Camera animation.
export struct CameraAnimation {
    /// Keyframes of camera position.
    std::vector<Keyframe<Vector3f>> position;
    /// Keyframes of camera matrix.
    std::vector<Keyframe<Matrix3f>> matrix;
};

/// Animation of single object.
export struct ObjectAnimation {
    /// Index of animated mesh in scene.
    UInt index{0};
    /// Keyframes of object position offset.
    std::vector<Keyframe<Vector3f>> position;
};

/// Animation of single light.
export struct LightAnimation {
    /// Index of animated light in scene.
    UInt index{0};
    /// Keyframes of light position.
    std::vector<Keyframe<Vector3f>> position;
};

/// Animation data for scene.
export class Animation {
  public:
    /// Duration of animation in seconds.
    Float duration = 0.0f;
    /// Frame rate of animation in frames per second.
    Float fps = 0.0f;
    /// Camera animation.
    CameraAnimation camera;
    /// Object animations.
    std::vector<ObjectAnimation> objects;
    /// Light animations.
    std::vector<LightAnimation> lights;
};

} // namespace rays
