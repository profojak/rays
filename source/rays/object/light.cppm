module;

export module rays:light;

import :point;
import :type;

namespace rays {

/// Base class for all lights.
export class Light {

  public:
    Light(Float intensity, Point3f position)
        : intensity_{intensity}, position_{position} {}

    /// Return light intensity.
    virtual Float Intensity() const { return intensity_; }
    /// Return light position.
    virtual Point3f Position() const { return position_; }

  private:
    /// Light intensity.
    Float intensity_;
    /// Light position.
    Point3f position_;
};

/// Point light.
export class PointLight : public Light {

  public:
    PointLight(Float intensity, Point3f position)
        : Light{intensity, position} {}
};

} // namespace rays
