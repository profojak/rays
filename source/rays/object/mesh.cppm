module;

#include "math/math.hpp" // IWYU pragma: keep

#include <cstddef>
#include <vector>

export module rays:mesh;

import :triangle;
import :type;
import :vector;

namespace rays {

/// Mesh.
export struct Mesh {

    /// Material index.
    Int material_index{-1};
    /// World-space offset.
    Vector3f position{};
    /// Vertices.
    std::vector<Vector3f> vertices;
    /// Triangles.
    std::vector<Triangle> triangles;
    /// Vertex normals.
    std::vector<Vector3f> vertex_normals;
    /// Face normals.
    std::vector<Vector3f> face_normals;
    /// Texture coordinates.
    std::vector<Vector3f> uvs;

    /// Recalculate normals.
    void RecalculateNormals() {
        face_normals.resize(triangles.size());
        for (UInt i = 0; i < triangles.size(); ++i) {
            const Triangle &triangle = triangles[i];
            const Vector3f &v0 = vertices[triangle.a];
            const Vector3f &v1 = vertices[triangle.b];
            const Vector3f &v2 = vertices[triangle.c];
            Vector3f normal = Vector3f::Cross(v1 - v0, v2 - v0);
            normal.Normalize();
            face_normals[i] = normal;
        }

        vertex_normals.assign(vertices.size(), Vector3f{0});
        for (UInt i = 0; i < triangles.size(); ++i) {
            const Triangle &triangle = triangles[i];
            const Vector3f &normal = face_normals[i];
            vertex_normals[triangle.a] += normal;
            vertex_normals[triangle.b] += normal;
            vertex_normals[triangle.c] += normal;
        }
        for (Vector3f &normal : vertex_normals) {
            normal.Normalize();
        }
    }
};

} // namespace rays
