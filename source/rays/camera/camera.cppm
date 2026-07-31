module;

#include <concepts>

export module rays:camera;

import :film;
import :vector;

namespace rays {

/// Camera that renders scene to film.
export template <std::floating_point T> class Camera {
  public:
    Camera(const Vector2u &film_size) : film_{film_size} {}

  private:
    /// Camera film.
    Film<T> film_;
};

} // namespace rays
