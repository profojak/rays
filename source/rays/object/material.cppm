module;

export module rays:material;

import :vector;

namespace rays {

/// Material.
export class Material {

  public:
    /// Type of material.
    enum class Type {
        Diffuse,
        Reflective,
    };

    Material(Type type, Vector3f albedo, bool smooth_shading)
        : type_(type), albedo_(albedo), smooth_shading_(smooth_shading) {}

  private:
    /// Type of material.
    Type type_;
    /// Albedo of material.
    Vector3f albedo_;
    /// Whether material is smooth shaded.
    bool smooth_shading_;
};

} // namespace rays
