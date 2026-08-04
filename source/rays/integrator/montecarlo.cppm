module;

#include <concepts>

export module rays:montecarlo;

import :film;
import :integrator;
import :mesh;
import :matrix;
import :pixel;
import :ray;
import :tile;
import :triangle;
import :type;
import :vector;

namespace rays {

/// Monte Carlo integrator that samples radiance along rays.
export template <std::floating_point T>
class MonteCarloIntegrator : public SamplingIntegrator<T> {

  public:
    MonteCarloIntegrator(const Vector2u &film_size_)
        : SamplingIntegrator<T>{film_size_} {}

  protected:
    /// Render single `Tile` of `Film`.
    void RenderTile(Tile<T> &tile, Film<T> &film) override {
        const auto &bounds = tile.Bounds();
        const auto resolution = film.GetResolution();
        for (UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
            for (UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
                auto &pixel = tile.PixelAt(x, y);
                const auto ray = GenerateRay(x, y, resolution);
                pixel = Intersect(ray) ? Pixel<T>{T{1}, T{1}, T{1}}
                                       : film.GetBackground();
            }
        }
        film.PutTile(tile);
    }

  private:
    /// Generate primary ray for pixel at film coordinates.
    [[nodiscard]] Ray3f GenerateRay(const UInt x, const UInt y,
                                    const Vector2u &resolution) const noexcept {
        const auto u =
            (static_cast<Float>(x) + 0.5f) * this->inverse_uv_[0] - 1.0f;
        const auto v =
            (static_cast<Float>(y) + 0.5f) * this->inverse_uv_[1] - 1.0f;

        Vector3f direction{u * this->aspect_, -v, -1.0f};
        direction = this->rotation_ * direction;
        direction.Normalize();

        return Ray3f{this->position_, direction};
    }

    /// Test ray against all meshes of scene.
    [[nodiscard]] bool Intersect(const Ray3f &ray) const noexcept {
        for (const auto &mesh : *this->meshes_) {
            const auto &vertices = mesh.vertices;
            for (const auto &triangle : mesh.triangles) {
                if (triangle.Intersect(ray, vertices[triangle.a],
                                       vertices[triangle.b],
                                       vertices[triangle.c])) {
                    return true;
                }
            }
        }
        return false;
    }
};

} // namespace rays
