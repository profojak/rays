module;

#include <memory>
#include <print>

#include "rays.hpp"

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

/// Return pointer to image data.
const void *Camera_ImageData() { return State::camera->ImageData(); }

/// Render scene to film.
void Camera_Render() { State::camera->Render(State::thread_pool); }

} // namespace rays

extern "C" { // C API
void Rays_Greet() { rays::Greet(); }

void Rays_Camera_Create(unsigned int width, unsigned int height) {
    rays::Camera_Create(width, height);
}

const void *Rays_Camera_ImageData() { return rays::Camera_ImageData(); }

void Rays_Camera_Render() { rays::Camera_Render(); }
}
