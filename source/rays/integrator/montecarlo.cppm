module;

#include "math/math.hpp" // IWYU pragma: keep

#include <algorithm>
#include <cmath>
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
                for (UInt i = 0; i < this->options_->samples_per_pixel; ++i) {
                    const auto ray = GenerateRay(x, y);
                    sum += Sample(ray, film.GetBackground());
                }
                sum /= static_cast<T>(this->options_->samples_per_pixel);
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

        // Shade based on material type.
        const auto type = mesh.material_index < 0
                              ? Material::Type::Diffuse
                              : materials[mesh.material_index].type;
        switch (type) {
        case Material::Type::Constant:
            return SampleConstant(albedo);
        case Material::Type::Reflective:
            return SampleReflective(ray, background, depth, hit, normal,
                                    albedo);
        case Material::Type::Refractive:
            return SampleRefractive(ray, background, depth, hit, normal,
                                    materials[mesh.material_index]);
        case Material::Type::Diffuse:
            return SampleDiffuse(hit, normal, background, albedo);
        }
        return background;
    }

    /// Constant material.
    [[nodiscard]] Vector3f
    SampleConstant(const Vector3f &albedo) const noexcept {
        return albedo;
    }

    /// Reflective material.
    [[nodiscard]] Vector3f
    SampleReflective(const Ray3f &ray, const Vector3f &background,
                     const UInt depth, const Vector3f &hit,
                     const Vector3f &normal,
                     const Vector3f &albedo) const noexcept {
        if (depth >= max_bounces_) {
            return background;
        }

        Vector3f reflection = normal;
        if (linalg::dot(reflection.View(), ray.direction.View()) > 0.0f) {
            reflection = -reflection;
        }
        return ReflectRay(ray, background, depth, hit, reflection) * albedo;
    }

    /// Refractive material.
    [[nodiscard]] Vector3f
    SampleRefractive(const Ray3f &ray, const Vector3f &background,
                     const UInt depth, const Vector3f &hit,
                     const Vector3f &normal,
                     const Material &material) const noexcept {
        if (depth >= max_bounces_) {
            return background;
        }

        // Whether ray is exiting material.
        const bool is_exiting =
            linalg::dot(normal.View(), ray.direction.View()) > 0.0f;
        Vector3f refraction = normal;
        if (is_exiting) {
            refraction = -refraction;
        }
        const Float eta = is_exiting ? material.index_of_refraction
                                     : 1.0f / material.index_of_refraction;
        const Float cos_theta =
            -linalg::dot(ray.direction.View(), refraction.View());
        const Float sin2_theta = eta * eta * (1.0f - cos_theta * cos_theta);

        // Total internal reflection.
        if (sin2_theta > 1.0f) {
            return ReflectRay(ray, background, depth, hit, refraction);
        }

        // Refracted ray.
        const Vector3f refracted_direction =
            ray.direction * eta +
            refraction * (eta * cos_theta - std::sqrt(1.0f - sin2_theta));
        const Vector3f refracted_color =
            Sample(Ray3f{hit - refraction * Epsilon, refracted_direction},
                   background, depth + 1);

        // Reflected ray.
        const Vector3f reflection_color =
            ReflectRay(ray, background, depth, hit, refraction);

        // Schlick Fresnel approximation.
        const Float fresnel = 0.5f * std::pow(1.0f - cos_theta, 5.0f);
        return reflection_color * fresnel + refracted_color * (1.0f - fresnel);
    }

    /// Diffuse material.
    [[nodiscard]] Vector3f
    SampleDiffuse(const Vector3f &hit, const Vector3f &normal,
                  const Vector3f &background,
                  const Vector3f &albedo) const noexcept {
        if (this->lights_->empty()) {
            return albedo;
        }

        // Contribution of single light.
        const auto sample_light = [&](const auto &light) -> Vector3f {
            Vector3f light_direction = light->Position() - hit;
            const Float distance =
                linalg::vector_two_norm(light_direction.View());
            light_direction.Normalize();
            const Float cosine = std::max(
                0.0f, linalg::dot(light_direction.View(), normal.View()));

            Ray3f shadow_ray{hit + normal * Epsilon, light_direction};
            const auto shadow_intersection =
                Intersect(shadow_ray, background, true);
            if (shadow_intersection && shadow_intersection->t < distance) {
                return Vector3f{0.0f};
            }

            const Float sphere_area =
                4.0f * std::numbers::pi_v<Float> * distance * distance;
            return albedo * light->Intensity() / sphere_area * cosine;
        };

        if (this->options_->sample_all_lights) {
            Vector3f radiance{0.0f};
            for (const auto &light : *this->lights_) {
                radiance += sample_light(light);
            }
            return radiance;
        } else {
            const auto &light =
                (*this->lights_)[Random::Range(this->lights_->size())];
            return sample_light(light) *
                   static_cast<Float>(this->lights_->size());
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

    /// Trace reflected ray at surface point.
    [[nodiscard]] Vector3f ReflectRay(const Ray3f &ray,
                                      const Vector3f &background,
                                      const UInt depth, const Vector3f &hit,
                                      const Vector3f &normal) const noexcept {
        const Vector3f reflection_direction =
            ray.direction -
            normal * (2.0f * linalg::dot(ray.direction.View(), normal.View()));
        return Sample(Ray3f{hit + normal * Epsilon, reflection_direction},
                      background, depth + 1);
    }

    /// Find closest intersection along ray across all meshes.
    [[nodiscard]] std::optional<TriangleIntersection3f>
    Intersect(const Ray3f &ray, const Vector3f &background,
              const bool ignore_refractive = false) const noexcept {
        std::optional<TriangleIntersection3f> closest_intersection;

        for (UInt i = 0; i < this->meshes_->size(); ++i) {
            const auto &mesh = (*this->meshes_)[i];
            if (ignore_refractive && mesh.material_index >= 0 &&
                (*this->materials_)[mesh.material_index].type ==
                    Material::Type::Refractive) {
                continue;
            }
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
