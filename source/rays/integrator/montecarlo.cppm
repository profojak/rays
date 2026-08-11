module;

#include "math/math.hpp" // IWYU pragma: keep

#include <algorithm>
#include <concepts>
#include <numbers>
#include <optional>

export module rays:montecarlo;

import :film;
import :integrator;
import :material;
import :mesh;
import :matrix;
import :pixel;
import :random;
import :ray;
import :tile;
import :triangle;
import :type;
import :vector;

namespace rays {

/// Monte Carlo integrator that samples radiance along rays.
export template <std::floating_point T>
class MonteCarloIntegrator : public SamplingIntegrator<T> {

  private:
    /// Number of samples per pixel.
    UInt samples_per_pixel_ = 4;
    /// Maximum number of reflection bounces.
    UInt max_bounces_ = 4;

  public:
    MonteCarloIntegrator(const Vector2u &film_size_)
        : SamplingIntegrator<T>{film_size_} {}

  protected:
    /// Render single `Tile` of `Film`.
    void RenderTile(Tile<T> &tile, Film<T> &film) override {
        const auto &bounds = tile.Bounds();
        for (UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
            for (UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
                auto &pixel = tile.PixelAt(x, y);
                Vector3f sum{0.0f};
                for (UInt i = 0; i < samples_per_pixel_; ++i) {
                    const auto ray = GenerateRay(x, y);
                    sum += Sample(ray, film.GetBackground());
                }
                sum /= static_cast<T>(samples_per_pixel_);
                pixel = sum;
            }
        }
        film.PutTile(tile);
    }

  private:
    /// Sample ray and return color.
    [[nodiscard]] Vector3f
    Sample(const Ray3f &ray,
           const Vector3f &background) const noexcept override {
        return Sample(ray, background, 0);
    }

    /// Sample ray at given bounce depth and return color.
    [[nodiscard]] Vector3f Sample(const Ray3f &ray, const Vector3f &background,
                                  const UInt depth) const noexcept {
        const auto intersection = Intersect(ray, background);
        if (!intersection) {
            return background;
        }

        const auto &mesh = (*this->meshes_)[(*intersection).mesh_index];
        const auto &triangle = mesh.triangles[(*intersection).triangle_index];
        const auto &materials = *this->materials_;
        const auto &v0 = mesh.vertices[triangle.a];
        const auto &v1 = mesh.vertices[triangle.b];
        const auto &v2 = mesh.vertices[triangle.c];

        // Albedo.
        const Vector3f albedo = mesh.material_index < 0
                                    ? Vector3f{1.0f}
                                    : materials[mesh.material_index].albedo;

        // Normal.
        Vector3f normal;
        if (mesh.material_index < 0 ||
            !materials[mesh.material_index].smooth_shading) {
            normal = mesh.face_normals[(*intersection).triangle_index];
        } else {
            const auto u = (*intersection).uv[0];
            const auto v = (*intersection).uv[1];
            normal = mesh.vertex_normals[triangle.a] * (1.0f - u - v) +
                     mesh.vertex_normals[triangle.b] * u +
                     mesh.vertex_normals[triangle.c] * v;
        }
        normal.Normalize();

        const Vector3f edge1 = v1 - v0;
        const Vector3f edge2 = v2 - v0;
        const Vector3f hit =
            v0 + edge1 * (*intersection).uv[0] + edge2 * (*intersection).uv[1];

        // Reflective material.
        if (mesh.material_index >= 0 &&
            materials[mesh.material_index].type == Material::Type::Reflective) {
            if (depth >= max_bounces_) {
                return background;
            }

            Vector3f reflection_normal = normal;
            if (linalg::dot(reflection_normal.View(), ray.direction.View()) >
                0.0f) {
                reflection_normal = -reflection_normal;
            }
            const Vector3f reflection =
                ray.direction -
                reflection_normal *
                    (2.0f * linalg::dot(ray.direction.View(),
                                        reflection_normal.View()));

            const Vector3f reflected_color =
                Sample(Ray3f{hit + reflection_normal * Epsilon, reflection},
                       background, depth + 1);
            return albedo * reflected_color;
        }

        if (!this->lights_->empty()) {
            const auto &light =
                (*this->lights_)[Random::Range(this->lights_->size())];
            Vector3f light_direction = light->Position() - hit;
            const Float distance =
                linalg::vector_two_norm(light_direction.View());
            light_direction.Normalize();
            Float cosine = std::max(
                0.0f, linalg::dot(light_direction.View(), normal.View()));

            Ray3f shadow_ray{hit + normal * Epsilon, light_direction};
            const auto shadow_intersection = Intersect(shadow_ray, background);
            if (shadow_intersection && shadow_intersection->t < distance) {
                return Vector3f{0.0f};
            } else {
                const Float sphere_area =
                    4.0f * std::numbers::pi_v<Float> * distance * distance;
                return albedo * light->Intensity() / sphere_area * cosine *
                       static_cast<Float>(this->lights_->size());
            }
        } else {
            return albedo;
        }
    }

    /// Generate primary ray for pixel at film coordinates.
    [[nodiscard]] Ray3f GenerateRay(const UInt x, const UInt y) const noexcept {
        const auto u = (static_cast<Float>(x) + 0.5f + Random::Unit()) *
                           this->inverse_uv_[0] -
                       1.0f;
        const auto v = (static_cast<Float>(y) + 0.5f + Random::Unit()) *
                           this->inverse_uv_[1] -
                       1.0f;

        Vector3f direction{u * this->aspect_, -v, -1.0f};
        direction = this->rotation_ * direction;
        direction.Normalize();

        return Ray3f{this->position_, direction};
    }

    /// Find closest intersection along ray across all meshes.
    [[nodiscard]] std::optional<TriangleIntersection3f>
    Intersect(const Ray3f &ray, const Vector3f &background) const noexcept {
        std::optional<TriangleIntersection3f> closest_intersection;

        for (UInt i = 0; i < this->meshes_->size(); ++i) {
            const auto &mesh = (*this->meshes_)[i];
            const auto &vertices = mesh.vertices;
            for (UInt j = 0; j < mesh.triangles.size(); ++j) {
                const auto &triangle = mesh.triangles[j];
                const auto intersection = triangle.Intersect(
                    ray, vertices[triangle.a], vertices[triangle.b],
                    vertices[triangle.c]);
                if (intersection &&
                    (!closest_intersection ||
                     intersection->t < closest_intersection->t)) {
                    closest_intersection = intersection;
                    closest_intersection->mesh_index = i;
                    closest_intersection->triangle_index = j;
                }
            }
        }
        if (!closest_intersection) {
            return std::nullopt;
        }

        return closest_intersection;
    }
};

} // namespace rays
