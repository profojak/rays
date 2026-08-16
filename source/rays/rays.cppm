module;

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
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
void LoadScene(const std::string &path, Rays_Scene_Type type) {
    const auto loader_type = static_cast<rays::Loader::Type>(type);
    switch (loader_type) {
    case Loader::Type::CRT:
        State::scene = CRTLoader::Load(path);
        break;
    }
}

/// Move camera relative to view direction.
bool MoveCamera(const Rays_Camera_MoveInput &input) {
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
bool RotateCamera(const Rays_Camera_RotateInput &input) {
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
Vector2u GetResolution() {
    return State::scene->GetCamera().GetFilm().GetResolution();
}

/// Return pointer to image data.
const void *ImageData() {
    return State::scene->GetCamera().GetFilm().ImageData();
}

/// Render scene to film.
void Render(Rays_Options &options) {
    State::scene->Render(State::thread_pool, options);
}

/// Check if camera render is in progress.
bool IsRendering() { return State::scene->GetCamera().IsRendering(); }

/// Block until camera render finishes.
void WaitForRender() { State::scene->GetCamera().WaitForRender(); }

/// Preview scene with fast rendering.
void Preview(unsigned long long time_budget, Rays_Options &options) {
    State::scene->Preview(State::thread_pool, time_budget, options);
}

/// Save camera image to `output.ppm` in given working directory.
void SaveImage(const std::string &working_directory) {
    if (working_directory.empty()) {
        std::println(std::cerr,
                     "Failed to save image: empty working directory!");
        return;
    }

    const auto &film = State::scene->GetCamera().GetFilm();
    const Vector2u resolution = film.GetResolution();
    const auto *image = static_cast<const UChar *>(film.ImageData());

    const std::filesystem::path path =
        std::filesystem::path(working_directory) / "output.ppm";
    std::ofstream file{path, std::ios::binary};
    if (!file) {
        std::println(std::cerr, "Failed to open output file: {}!",
                     path.string());
        return;
    }

    file << "P6\n" << resolution[0] << ' ' << resolution[1] << "\n255\n";
    for (UInt y = 0; y < resolution[1]; ++y) {
        for (UInt x = 0; x < resolution[0]; ++x) {
            const auto *pixel = image + (y * resolution[0] + x) * 4;
            file.put(static_cast<char>(pixel[0]));
            file.put(static_cast<char>(pixel[1]));
            file.put(static_cast<char>(pixel[2]));
        }
    }
    std::println("Saved image to {}", path.string());
}

/// Return duration of animation in seconds.
float AnimationDuration() { return State::scene->GetAnimation().duration; }

/// Return frames per second of animation.
float AnimationFPS() { return State::scene->GetAnimation().fps; }

/// Render single frame of animation.
void RenderAnimationFrame(unsigned int frame, Rays_Options &options) {
    State::scene->SetFrame(frame);
    State::scene->Render(State::thread_pool, options);
}

/// Preview single frame of animation.
void PreviewAnimationFrame(unsigned int frame, unsigned long long time_budget,
                           Rays_Options &options) {
    State::scene->SetFrame(frame);
    State::scene->Preview(State::thread_pool, time_budget, options);
}

} // namespace rays

extern "C" { // C API
void Rays_Greet() { rays::Greet(); }

void Rays_LoadScene(const char *path, Rays_Scene_Type type) {
    rays::LoadScene(path, type);
}

bool Rays_MoveCamera(const Rays_Camera_MoveInput &input) {
    return rays::MoveCamera(input);
}

bool Rays_RotateCamera(const Rays_Camera_RotateInput &input) {
    return rays::RotateCamera(input);
}

void Rays_GetResolution(unsigned int *width, unsigned int *height) {
    auto res = rays::GetResolution();
    *width = res[0];
    *height = res[1];
}

const void *Rays_ImageData() { return rays::ImageData(); }

void Rays_Render(Rays_Options &options) { rays::Render(options); }

bool Rays_IsRendering() { return rays::IsRendering(); }

void Rays_WaitForRender() { rays::WaitForRender(); }

void Rays_Preview(unsigned long long time_budget, Rays_Options &options) {
    rays::Preview(time_budget, options);
}

void Rays_SaveImage(const char *working_directory) {
    rays::SaveImage(working_directory ? working_directory : "");
}

float Rays_AnimationDuration() { return rays::AnimationDuration(); }

float Rays_AnimationFPS() { return rays::AnimationFPS(); }

void Rays_RenderAnimationFrame(unsigned int frame, Rays_Options &options) {
    rays::RenderAnimationFrame(frame, options);
}

void Rays_PreviewAnimationFrame(unsigned int frame,
                                unsigned long long time_budget,
                                Rays_Options &options) {
    rays::PreviewAnimationFrame(frame, time_budget, options);
}
}
