module;

#include <random>

export module rays:random;

import :type;

namespace rays {

/// Pseudo-random number generator.
export class Random {
  public:
    /// Return uniform random `Float` in [0, 1).
    [[nodiscard]] static Float Unit() noexcept {
        return std::uniform_real_distribution<Float>{0.0f, 1.0f}(engine_);
    }

  private:
    /// Random engine, one per thread.
    static inline thread_local std::mt19937 engine_{std::random_device{}()};
};

} // namespace rays
