module;

#include <concepts>

export module rays:concepts;

namespace rays {

/// Concept for arithmetic types.
export template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

} // namespace rays
