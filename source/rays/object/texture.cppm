module;

#include <string>
#include <utility>

export module rays:texture;

import :type;
import :vector;

namespace rays {

/// Texture.
export struct Texture {

    /// Type of texture.
    enum class Type {
        Albedo,
    };

    Texture(std::string name, Type type, Vector3f albedo)
        : name{std::move(name)}, type{type}, albedo{albedo} {}

    /// Name of texture.
    std::string name;
    /// Type of texture.
    Type type;
    /// Albedo color of texture.
    Vector3f albedo;
};

} // namespace rays
