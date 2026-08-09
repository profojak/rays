module;

export module rays:light;

import :type;
import :vector;

namespace rays {

/// Base class for all lights.
export class Light {

  public:
    Light(Float intensity, Vector3f position)
        : intensity_{intensity}, position_{position} {}

    /// Return light intensity.
    virtual Float Intensity() const { return intensity_; }
    /// Return light position.
    virtual Vector3f Position() const { return position_; }

  private:
    /// Light intensity.
    Float intensity_;
    /// Light position.
    Vector3f position_;
};

/// Point light.
export class PointLight : public Light {

  public:
    PointLight(Float intensity, Vector3f position)
        : Light{intensity, position} {}
};

} // namespace rays
