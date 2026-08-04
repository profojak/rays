module;

#include <memory>
#include <print>
#include <string>

#include "rays.hpp"

export module rays;

import :camera;
import :loader;
import :state;
import :type;
import :vector;

export namespace rays {

/// Greet folks from Chaos!
void Greet() { std::println("Hello, Chaos!"); }

/// Load scene from file.
void Scene_Load(const std::string &path, Loader::Type type) {
    switch (type) {
    case Loader::Type::CRT:
        State::scene = CRTLoader::Load(path);
        break;
    }
}

/// Get camera resolution.
Vector2u Camera_GetResolution() {
    return State::scene->GetCamera().GetFilm().GetResolution();
}

/// Return pointer to image data.
const void *Camera_ImageData() {
    return State::scene->GetCamera().GetFilm().ImageData();
}

/// Render scene to film.
void Camera_Render() { State::scene->Render(State::thread_pool); }

} // namespace rays

extern "C" { // C API
void Rays_Greet() { rays::Greet(); }

void Rays_Scene_Load(const char *path, Rays_Scene_Type type) {
    rays::Scene_Load(path, static_cast<rays::Loader::Type>(type));
}

void Rays_Camera_GetResolution(unsigned int *width, unsigned int *height) {
    auto res = rays::Camera_GetResolution();
    *width = res[0];
    *height = res[1];
}

const void *Rays_Camera_ImageData() { return rays::Camera_ImageData(); }

void Rays_Camera_Render() { rays::Camera_Render(); }
}
