module;

#include <cmath>
#include <memory>
#include <print>
#include <string>

#include "rays.h"

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

/// Move camera relative to view direction.
bool Camera_Move(const Rays_Camera_MoveInput &input) {
    auto &camera = State::scene->GetCamera();
    auto position = camera.GetPosition();
    const Matrix3f rotation = camera.GetRotation();

    // Camera local axes mapped to world space.
    const Vector3f forward = rotation * Vector3f{0.0f, 0.0f, -1.0f};
    const Vector3f right = rotation * Vector3f{1.0f, 0.0f, 0.0f};
    const Vector3f up = rotation * Vector3f{0.0f, 1.0f, 0.0f};

    constexpr Float move_step = 0.1f;
    bool moved = false;
    if (input.forward) {
        position += forward * move_step;
        moved = true;
    }
    if (input.backward) {
        position -= forward * move_step;
        moved = true;
    }
    if (input.left) {
        position -= right * move_step;
        moved = true;
    }
    if (input.right) {
        position += right * move_step;
        moved = true;
    }
    if (input.up) {
        position += up * move_step;
        moved = true;
    }
    if (input.down) {
        position -= up * move_step;
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

/// Check if camera render is in progress.
bool Camera_IsRendering() { return State::scene->GetCamera().IsRendering(); }

/// Block until camera render finishes.
void Camera_WaitForRender() { State::scene->GetCamera().WaitForRender(); }

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

bool Rays_Camera_IsRendering() { return rays::Camera_IsRendering(); }

void Rays_Camera_WaitForRender() { rays::Camera_WaitForRender(); }

void Rays_Camera_Preview(unsigned long long time_budget) {
    rays::Camera_Preview(time_budget);
}
}
