module;

#include <vector>

export module rays:mesh;

import :triangle;
import :vector;

namespace rays {

/// Mesh.
export struct Mesh {

    /// Vertices.
    std::vector<Vector3f> vertices;
    /// Triangles.
    std::vector<Triangle> triangles;
};

} // namespace rays
