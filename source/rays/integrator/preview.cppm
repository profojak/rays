module;

#include "math/math.hpp" // IWYU pragma: keep

#include <cmath>
#include <concepts>
#include <limits>
#include <optional>
#include <variant>

export module rays:preview;

import :integrator;
import :light;
import :material;
import :ray;
import :sphere;
import :type;
import :vector;

namespace rays {

export template <std::floating_point T>
class PreviewIntegrator : public SamplingIntegrator<T> {

  private:
    UInt pass_{0};

  public:
    PreviewIntegrator(const Vector2u &film_size_)
        : SamplingIntegrator<T>{film_size_} {}

    /// Reset integrator state.
    void Reset() { pass_ = 0; }

    /// Increment pass counter.
    void Pass() { ++pass_; }

  protected:
    /// Render single `Tile` of `Film`.
    void RenderTile(Tile<T> &tile, Film<T> &film) override {
        const auto &bounds = tile.Bounds();
        const auto width = bounds.Size(0);
        const auto height = bounds.Size(1);
        const auto max_dim = width > height ? width : height;
        const auto requested_dim = UInt{1} << (pass_ - 1);
        const auto dim = requested_dim < max_dim ? requested_dim : max_dim;
        const auto fully_refined = requested_dim > max_dim;
        for (UInt y = 0; y < dim; ++y) {
            const auto min_y = bounds.min[1] + y * height / dim;
            const auto max_y = bounds.min[1] + (y + 1) * height / dim;
            for (UInt x = 0; x < dim; ++x) {
                const auto min_x = bounds.min[0] + x * width / dim;
                const auto max_x = bounds.min[0] + (x + 1) * width / dim;

                Pixel<T> sample;
                if (fully_refined || (pass_ > 1 && x % 2 == 0 && y % 2 == 0)) {
                    sample = film.PixelAt(min_x, min_y);
                } else {
                    const auto ray = GenerateRay(min_x, min_y);
                    sample = Sample(ray, film.GetBackground());
                }

                for (UInt pixel_y = min_y; pixel_y < max_y; ++pixel_y) {
                    for (UInt pixel_x = min_x; pixel_x < max_x; ++pixel_x) {
                        tile.PixelAt(pixel_x, pixel_y) = sample;
                    }
                }
            }
        }
        film.PutTile(tile);
    }

  private:
    /// Sample ray and return color.
    [[nodiscard]] Vector3f
    Sample(const Ray3f &ray,
           const Vector3f &background) const noexcept override {
        const auto intersection = Intersect(ray);
        if (const auto *triangle =
                std::get_if<TriangleIntersection3f>(&intersection)) {
            return Shade(ray, *triangle);
        }
        if (std::holds_alternative<SphereIntersection3f>(intersection)) {
            return Vector3f{1.0f};
        }
        return background;
    }

    /// Generate primary ray for pixel at film coordinates.
    [[nodiscard]] Ray3f GenerateRay(const UInt x, const UInt y) const noexcept {
        const auto u = static_cast<Float>(x) * this->inverse_uv_[0] - 1.0f;
        const auto v = static_cast<Float>(y) * this->inverse_uv_[1] - 1.0f;

        Vector3f direction{u * this->aspect_, -v, -1.0f};
        direction = this->rotation_ * direction;
        direction.Normalize();

        return Ray3f{this->position_, direction};
    }

    /// Find closest intersection along ray across all objects.
    [[nodiscard]] std::variant<std::monostate, TriangleIntersection3f,
                               SphereIntersection3f>
    Intersect(const Ray3f &ray) const noexcept {
        std::optional<TriangleIntersection3f> triangle_hit;
        if (this->options_->use_bvh) {
            triangle_hit = this->bvh_.Intersect(ray);

        } else {
            for (UInt i = 0; i < this->meshes_->size(); ++i) {
                const auto &mesh = (*this->meshes_)[i];
                const auto &vertices = mesh.vertices;
                const bool back_face_culling =
                    mesh.material_index >= 0 &&
                    (*this->materials_)[mesh.material_index].back_face_culling;
                for (UInt j = 0; j < mesh.triangles.size(); ++j) {
                    const auto &triangle = mesh.triangles[j];
                    const auto intersection = triangle.Intersect(
                        ray, vertices[triangle.a], vertices[triangle.b],
                        vertices[triangle.c], back_face_culling);
                    if (intersection &&
                        (!triangle_hit || intersection->t < triangle_hit->t)) {
                        triangle_hit = intersection;
                        triangle_hit->mesh_index = i;
                        triangle_hit->triangle_index = j;
                    }
                }
            }
        }

        // Light spheres closer than any triangle.
        const Float triangle_t = triangle_hit
                                     ? triangle_hit->t
                                     : std::numeric_limits<Float>::infinity();
        for (const auto &light : *this->lights_) {
            if (const auto *point_light =
                    dynamic_cast<const PointLight *>(light.get())) {
                const auto intersection =
                    point_light->GetSphere().Intersect(ray);
                if (intersection && intersection->t < triangle_t) {
                    return *intersection;
                }
            }
        }

        if (triangle_hit) {
            return *triangle_hit;
        }
        return std::monostate{};
    }

    /// Shade triangle facing toward camera.
    [[nodiscard]] Pixel<T>
    Shade(const Ray3f &ray,
          const TriangleIntersection3f &intersection) const noexcept {
        const auto &mesh = (*this->meshes_)[intersection.mesh_index];
        Vector3f normal = mesh.face_normals[intersection.triangle_index];

        if (linalg::dot(normal.View(), ray.direction.View()) > 0.0f) {
            normal = -normal;
        }
        const Float shade = -linalg::dot(normal.View(), ray.direction.View());
        const auto color = MapToRGB(intersection.mesh_index);
        return Pixel<T>{color[0] * shade, color[1] * shade, color[2] * shade};
    }

    /// Map mesh index to RGB color.
    [[nodiscard]] Vector3f MapToRGB(const UInt mesh_index) const noexcept {
        const auto hue = static_cast<Float>(mesh_index) * 0.6180339887498948f;
        const Float h = (hue - std::floor(hue)) * 6.0f;
        const Float x = 1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f);
        if (h < 1.0f)
            return {1.0f, x, 0.0f};
        if (h < 2.0f)
            return {x, 1.0f, 0.0f};
        if (h < 3.0f)
            return {0.0f, 1.0f, x};
        if (h < 4.0f)
            return {0.0f, x, 1.0f};
        if (h < 5.0f)
            return {x, 0.0f, 1.0f};
        return {1.0f, 0.0f, x};
    }
};

} // namespace rays
