module;

#include <memory>
#include <print>

export module rays;

import :camera;
import :state;
import :type;
import :vector;

export namespace rays {

/// Greet folks from Chaos!
void Greet() { std::println("Hello, Chaos!"); }

/// Create camera with given film size.
void Camera_Create(unsigned int width, unsigned int height) {
    State::camera = std::make_unique<Camera<float>>(Vector2u{width, height});
}

} // namespace rays

extern "C" { // C API
void Rays_Greet() { rays::Greet(); }

void Rays_Camera_Create(unsigned int width, unsigned int height) {
    rays::Camera_Create(width, height);
}
}
