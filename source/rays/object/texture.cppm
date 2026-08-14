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
        Edges,
    };

    Texture(std::string name, Type type, Vector3f albedo, Vector3f edge_color,
            Vector3f inner_color, Float edge_width)
        : name{std::move(name)}, type{type}, albedo{albedo},
          edge_color{edge_color}, inner_color{inner_color},
          edge_width{edge_width} {}

    /// Name of texture.
    std::string name;
    /// Type of texture.
    Type type;
    /// Albedo color of texture.
    Vector3f albedo;
    /// Color of texture edges.
    Vector3f edge_color;
    /// Color inside texture edges.
    Vector3f inner_color;
    /// Width of texture edges.
    Float edge_width;
};

} // namespace rays
