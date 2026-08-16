module;

#include <numbers>
#include <random>

export module rays:random;

import :type;
import :vector;

namespace rays {

/// Pseudo-random number generator.
export class Random {
  public:
    /// Return uniform random `Float` in [0, 1).
    [[nodiscard]] static Float Unit() noexcept {
        return std::uniform_real_distribution<Float>{0.0f, 1.0f}(engine_);
    }

    /// Return uniform random `UInt` in [0, `range`).
    [[nodiscard]] static UInt Range(const UInt range) noexcept {
        return std::uniform_int_distribution<UInt>{0, range - 1}(engine_);
    }

    /// Return uniform random sample on unit disk.
    [[nodiscard]] static Vector2f UniformDisk() noexcept {
        const Vector2f sample{Unit(), Unit()};
        const Float r = std::sqrt(sample[0]);
        const Float theta = 2.0f * std::numbers::pi_v<Float> * sample[1];
        return Vector2f{r * std::cos(theta), r * std::sin(theta)};
    }

    /// Probability density function for uniform disk sampling.
    [[nodiscard]] static Float UniformDiskPDF(const Vector2f& sample) noexcept {
        return sample.Length() <= 1.0f ? std::numbers::inv_pi_v<Float> : 0.0f;
    }

    /// Return cosine-weighted uniform random sample on unit hemisphere.
    [[nodiscard]] static Vector3f CosineHemisphere() noexcept {
        const Vector2f sample = UniformDisk();
        const Float z = std::sqrt(std::max(0.0f, 1.0f - sample[0] * sample[0] - sample[1] * sample[1]));
        return Vector3f{sample[0], sample[1], z};
    }

    /// Probability density function for cosine-weighted uniform hemisphere sampling.
    [[nodiscard]] static Float CosineHemispherePDF(const Vector3f& sample) noexcept {
        return sample[2] >= 0.0f ? sample[2] * std::numbers::inv_pi_v<Float> : 0.0f;
    }


  private:
    /// Random engine, one per thread.
    static inline thread_local std::mt19937 engine_{std::random_device{}()};
};

} // namespace rays
