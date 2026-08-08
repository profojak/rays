module;

#include <cmath>
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

/// Move camera based on input.
bool Camera_Move(const Rays_Camera_MoveInput &input) {
    auto &camera = State::scene->GetCamera();
    auto position = camera.GetPosition();
    bool moved = false;
    if (input.forward) {
        position[2] -= 0.1f;
        moved = true;
    }
    if (input.backward) {
        position[2] += 0.1f;
        moved = true;
    }
    if (input.left) {
        position[0] -= 0.1f;
        moved = true;
    }
    if (input.right) {
        position[0] += 0.1f;
        moved = true;
    }
    if (input.up) {
        position[1] += 0.1f;
        moved = true;
    }
    if (input.down) {
        position[1] -= 0.1f;
        moved = true;
    }
    camera.SetPosition(position);
    return moved;
}

/// Rotate camera based on yaw and pitch deltas in radians.
bool Camera_Rotate(const Rays_Camera_RotateInput &input) {
    if (input.yaw == 0.0f && input.pitch == 0.0f) {
        return false;
    }

    auto &camera = State::scene->GetCamera();

    const Float cos_yaw = std::cos(input.yaw);
    const Float sin_yaw = std::sin(input.yaw);
    // Yaw rotates camera around world up axis, positive turns right.
    const Matrix3f yaw_rotation{cos_yaw, 0.0f,    -sin_yaw, 0.0f,   1.0f,
                                0.0f,    sin_yaw, 0.0f,     cos_yaw};

    const Float cos_pitch = std::cos(input.pitch);
    const Float sin_pitch = std::sin(input.pitch);
    // Pitch rotates camera around its local right axis, positive looks up.
    const Matrix3f pitch_rotation{1.0f, 0.0f,      0.0f,
                                  0.0f, cos_pitch, -sin_pitch,
                                  0.0f, sin_pitch, cos_pitch};

    camera.SetRotation(yaw_rotation * camera.GetRotation() * pitch_rotation);
    return true;
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

/// Preview scene with fast rendering.
void Camera_Preview(unsigned long long time_budget) {
    State::scene->Preview(State::thread_pool, time_budget);
}

} // namespace rays

extern "C" { // C API
void Rays_Greet() { rays::Greet(); }

void Rays_Scene_Load(const char *path, Rays_Scene_Type type) {
    rays::Scene_Load(path, static_cast<rays::Loader::Type>(type));
}

bool Rays_Camera_Move(const Rays_Camera_MoveInput &input) {
    return rays::Camera_Move(input);
}

bool Rays_Camera_Rotate(const Rays_Camera_RotateInput &input) {
    return rays::Camera_Rotate(input);
}

void Rays_Camera_GetResolution(unsigned int *width, unsigned int *height) {
    auto res = rays::Camera_GetResolution();
    *width = res[0];
    *height = res[1];
}

const void *Rays_Camera_ImageData() { return rays::Camera_ImageData(); }

void Rays_Camera_Render() { rays::Camera_Render(); }

void Rays_Camera_Preview(unsigned long long time_budget) {
    rays::Camera_Preview(time_budget);
}
}
