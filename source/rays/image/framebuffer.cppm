module;

#include <utility>

export module rays:framebuffer;

import :image;
import :vector;

namespace rays {

/// Frame buffer for rendering and writing to image.
export template <ImageFormat F> class FrameBuffer {
  public:
    FrameBuffer(const Vector2u &resolution)
        : double_buffer_{Image<F>{resolution}, Image<F>{resolution}} {}

  private:
    /// Double buffer for rendering.
    std::pair<Image<F>, Image<F>> double_buffer_;
};

} // namespace rays
