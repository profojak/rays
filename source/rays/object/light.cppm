module;

export module rays:light;

import :type;
import :vector;

namespace rays {

/// Abstract base class for all lights.
export class Light {

  public:
    virtual ~Light() = 0;

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

Light::~Light() = default;

/// Point light.
export class PointLight : public Light {};

} // namespace rays
