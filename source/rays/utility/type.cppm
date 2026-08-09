module;

#include <concepts>
#include <cstdint>
#if __has_include(<stdfloat>)
#include <stdfloat>
#endif

export module rays:type;

namespace rays {

#if __has_include(<stdfloat>)
export using Float = std::float32_t;
export using Double = std::float64_t;
#else
export using Float = float;
export using Double = double;
static_assert(sizeof(Float) == 4, "Float must be 4 bytes");
static_assert(sizeof(Double) == 8, "Double must be 8 bytes");
#endif

export using UChar = std::uint8_t;
export using Int = std::int32_t;
export using UInt = std::uint32_t;

/// Concept for arithmetic types.
export template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

export Float Epsilon = 1e-5f;

} // namespace rays
