module;

#include <cmath>
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
    MonteCarloIntegrator() = default;

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
        const auto aspect = static_cast<Float>(resolution[0]) /
                            static_cast<Float>(resolution[1]);
        const auto u = (static_cast<Float>(x) + Float{0.5}) /
                           static_cast<Float>(resolution[0]) * Float{2} -
                       Float{1};
        const auto v = (static_cast<Float>(y) + Float{0.5}) /
                           static_cast<Float>(resolution[1]) * Float{2} -
                       Float{1};

        Vector3f direction{u * aspect, -v, Float{-1}};
        direction = this->rotation_ * direction;

        const auto length = std::sqrt(direction[0] * direction[0] +
                                      direction[1] * direction[1] +
                                      direction[2] * direction[2]);
        direction /= length;

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
