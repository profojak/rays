module;

#include <utility>

export module rays:light;

import :point;
import :sphere;
import :type;

namespace rays {

/// Base class for all lights.
export class Light {

  public:
    Light(Float intensity, Point3f position)
        : intensity_{intensity}, position_{position} {}

    virtual ~Light() = default;

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
        : Light{intensity, position}, sphere_{position, intensity * 0.001f} {}

    /// Return light sphere for visualization.
    template <typename Self> [[nodiscard]] auto &GetSphere(this Self &&self) {
        return std::forward<Self>(self).sphere_;
    }

  private:
    /// Sphere representing light source for visualization.
    Sphere sphere_;
};

} // namespace rays
