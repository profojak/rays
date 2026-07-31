module;

#include <concepts>
#include <cstddef>
#include <mutex>
#include <vector>

export module rays:film;

import :image;
import :vector;
import :pixel;
import :tile;

export namespace rays {

/// 2D raster of accumulated `Pixel` samples.
template <std::floating_point T> class Film {

  public:
    Film(const Vector2u &resolution)
        : resolution_{resolution}, film_{resolution[0] * resolution[1]},
          image_{resolution} {}

    /// Return reference to pixel.
    template <typename Self>
    [[nodiscard]] auto &PixelAt(this Self &&self, UInt x, UInt y) {
        return self.film_[y * self.resolution_[0] + x];
    }

    /// Put tile into film.
    void PutTile(const Tile<T> &tile) {
        std::lock_guard<std::mutex> lock{mutex_};
        const auto &bounds = tile.Bounds();
        for (UInt y = bounds.min[1]; y < bounds.max[1]; ++y) {
            for (UInt x = bounds.min[0]; x < bounds.max[0]; ++x) {
                PixelAt(x, y) = tile.PixelAt(x, y);
            }
        }
    }

    /// Return pointer to image data.
    [[nodiscard]] const void *ImageData() const noexcept {
        using ImageType = decltype(image_);
        using ChannelType = typename ImageType::element_type;
        constexpr auto channels = ImageType::channels;

        auto *source = image_.Data();
        const auto count = film_.size();
        const auto Clamp = [](T v) noexcept -> ChannelType {
            if (v < T{0}) {
                v = T{0};
            } else if (v > T{1}) {
                v = T{1};
            }
            return static_cast<ChannelType>(v * T{255} + T{0.5});
        };

        std::lock_guard<std::mutex> lock{mutex_};
        for (std::size_t i = 0; i < count; ++i) {
            const auto &pixel = film_[i];
            const auto offset = i * channels;
            source[offset + 0] = Clamp(pixel.R());
            source[offset + 1] = Clamp(pixel.G());
            source[offset + 2] = Clamp(pixel.B());
            source[offset + 3] = ChannelType{255};
        }
        return source;
    }

  private:
    /// Film resolution.
    Vector2u resolution_{};
    /// Accumulated `Pixel` samples.
    std::vector<Pixel<T>> film_{};

    /// Image for rendering to screen.
    mutable Image<ImageFormat::R8G8B8A8> image_;
    /// Guard against concurrent render writers.
    mutable std::mutex mutex_;
};

} // namespace rays
