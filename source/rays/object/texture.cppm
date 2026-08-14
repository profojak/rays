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
        Checker,
    };

    Texture(std::string name, Type type, Vector3f albedo, Vector3f edge_color,
            Vector3f inner_color, Float edge_width, Vector3f color_A,
            Vector3f color_B, Float square_size)
        : name{std::move(name)}, type{type}, albedo{albedo},
          edge_color{edge_color}, inner_color{inner_color},
          edge_width{edge_width}, color_A{color_A}, color_B{color_B},
          square_size{square_size} {}

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
    /// First color of checker texture.
    Vector3f color_A;
    /// Second color of checker texture.
    Vector3f color_B;
    /// Size of checker square.
    Float square_size;
};

} // namespace rays
