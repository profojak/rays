module;

#include <concepts>
#include <limits>

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
        const auto resolution = film.GetResolution();
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
                    const auto ray = GenerateRay(min_x, min_y, resolution);
                    sample = Intersect(ray) ? Pixel<T>{T{1}, T{1}, T{1}}
                                            : film.GetBackground();
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
    /// Generate primary ray for pixel at film coordinates.
    [[nodiscard]] Ray3f GenerateRay(const UInt x, const UInt y,
                                    const Vector2u &resolution) const noexcept {
        const auto u = static_cast<Float>(x) * this->inverse_uv_[0] - 1.0f;
        const auto v = static_cast<Float>(y) * this->inverse_uv_[1] - 1.0f;

        Vector3f direction{u * this->aspect_, -v, -1.0f};
        direction = this->rotation_ * direction;
        direction.Normalize();

        return Ray3f{this->position_, direction};
    }

    /// Test ray against all meshes of scene.
    [[nodiscard]] bool Intersect(const Ray3f &ray) const noexcept {
        auto t = std::numeric_limits<Float>::infinity();
        for (const auto &mesh : *this->meshes_) {
            const auto &vertices = mesh.vertices;
            for (const auto &triangle : mesh.triangles) {
                if (triangle.Intersect(ray, vertices[triangle.a],
                                       vertices[triangle.b],
                                       vertices[triangle.c], t)) {
                    return true;
                }
            }
        }
        return false;
    }
};

} // namespace rays
