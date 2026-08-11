module;

export module rays:material;

import :vector;

namespace rays {

/// Material.
export struct Material {

    /// Type of material.
    enum class Type {
        Diffuse,
        Reflective,
    };

    Material(Type type, Vector3f albedo, bool smooth_shading)
        : type(type), albedo(albedo), smooth_shading(smooth_shading) {}

    /// Type of material.
    Type type;
    /// Albedo of material.
    Vector3f albedo;
    /// Whether material is smooth shaded.
    bool smooth_shading;
};

} // namespace rays
