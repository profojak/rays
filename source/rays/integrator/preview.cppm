module;

#include "math/math.hpp" // IWYU pragma: keep

#include <cmath>
#include <concepts>
#include <optional>

export module rays:preview;

import :integrator;
import :type;

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
        if (const auto intersection = Intersect(ray)) {
            return Shade(ray, *intersection);
        } else {
            return background;
        }
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

    /// Find closest intersection along ray across all meshes.
    [[nodiscard]] std::optional<TriangleIntersection3f>
    Intersect(const Ray3f &ray) const noexcept {
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

    /// Shade triangle facing toward camera.
    [[nodiscard]] Pixel<T>
    Shade(const Ray3f &ray,
          const TriangleIntersection3f &intersection) const noexcept {
        const auto &mesh = (*this->meshes_)[intersection.mesh_index];
        const auto &triangle = mesh.triangles[intersection.triangle_index];
        const auto &v0 = mesh.vertices[triangle.a];
        const auto &v1 = mesh.vertices[triangle.b];
        const auto &v2 = mesh.vertices[triangle.c];

        const Vector3f edge1 = v1 - v0;
        const Vector3f edge2 = v2 - v0;
        Vector3f normal = Vector3f::Cross(edge1, edge2);
        normal.Normalize();

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
