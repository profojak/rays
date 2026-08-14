module;

#include <variant>

export module rays:material;

import :type;
import :vector;

namespace rays {

/// Material.
export struct Material {

    /// Type of material.
    enum class Type {
        Constant,
        Reflective,
        Refractive,
        Diffuse,
    };

    Material(Type type, std::variant<Vector3f, UInt> albedo,
             bool smooth_shading, Float index_of_refraction = 1.5f)
        : type{type}, albedo{albedo}, smooth_shading{smooth_shading},
          index_of_refraction{index_of_refraction} {}

    /// Type of material.
    Type type;
    /// Albedo of material, or texture index.
    std::variant<Vector3f, UInt> albedo;
    /// Whether material is smooth shaded.
    bool smooth_shading;
    /// Index of refraction for refractive materials.
    Float index_of_refraction;
};

} // namespace rays
