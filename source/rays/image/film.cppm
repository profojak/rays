module;

#include <concepts>
#include <vector>

export module rays:film;

import :framebuffer;
import :image;
import :vector;
import :pixel;

export namespace rays {

/// 2D raster of accumulated `Pixel` samples.
template <std::floating_point T> class Film {

  public:
    Film(const Vector2u &resolution)
        : resolution_{resolution}, film_{resolution[0] * resolution[1]},
          frame_buffer_{resolution} {}

  private:
    /// Film resolution.
    Vector2u resolution_{};
    /// Accumulated `Pixel` samples.
    std::vector<Pixel<T>> film_{};
    /// Framebuffer for rendering to screen and writing to image.
    FrameBuffer<ImageFormat::R8G8B8A8> frame_buffer_;
};

} // namespace rays
