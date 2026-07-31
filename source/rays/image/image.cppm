module;

#include <algorithm>
#include <vector>

export module rays:image;

import :type;
import :vector;

namespace rays {

/// Image format.
export enum class ImageFormat {
    R8G8B8A8 /// RGB with alpha channel, 8 bits per channel.
};

/// Traits for image format.
template <ImageFormat F> struct ImageFormatTraits;

/// Traits for `R8G8B8A8` image format.
template <> struct ImageFormatTraits<ImageFormat::R8G8B8A8> {
    using element_type = UChar;
    static constexpr std::size_t channels = 4;
};

/// Image.
export template <ImageFormat F> class Image {
  public:
    /// Element type of image data.
    using element_type = typename ImageFormatTraits<F>::element_type;

    Image(const Vector2u &resolution) : resolution_{resolution} {
        constexpr std::size_t channels = ImageFormatTraits<F>::channels;
        data_.resize(resolution[0] * resolution[1] * channels);
        std::fill(data_.begin(), data_.end(), 0);
    }

  private:
    /// Resolution of image.
    Vector2u resolution_;
    /// Image data.
    std::vector<element_type> data_;
};

} // namespace rays
