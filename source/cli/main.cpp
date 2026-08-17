#include <chrono>
#include <filesystem>
#include <format>
#include <print>

#include "rays.h"

import rays;

int main(int argc, char *argv[]) {
    rays::Greet();

    if (argc < 2) {
        std::println("Usage: {} <scene>", argv[0]);
        return 1;
    }

    rays::LoadScene(argv[1], Rays_Scene_Type_CRT);

    Rays_Options options{.sample_all_lights = true,
                         .use_bvh = true,
                         .global_illumination = true,
                         .samples_per_pixel = 2};

    const auto animation_duration = rays::AnimationDuration();

    // No animation.
    if (animation_duration <= 0.0f) {
        auto start = std::chrono::high_resolution_clock::now();
        std::println("Rendering...");

        rays::Render(options);
        rays::WaitForRender();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::println("Rendered in {:.2f}ms", elapsed.count() * 1000);

        rays::SaveImage(std::filesystem::current_path().string());

    } else {
        const auto animation_fps = rays::AnimationFPS();
        const unsigned int frame_count = std::max(
            1u, static_cast<unsigned int>(animation_duration * animation_fps));

        const auto video_dir = std::filesystem::current_path() / "video";
        std::filesystem::create_directories(video_dir);

        auto start = std::chrono::high_resolution_clock::now();
        std::println("Rendering animation of {} frames...", frame_count);

        for (unsigned int frame = 0; frame < frame_count; ++frame) {
            rays::RenderAnimationFrame(frame, options);
            rays::WaitForRender();
            rays::SaveImageTo(
                (video_dir / std::format("frame_{:04d}.ppm", frame)).string());
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::println("Rendered in {:.2f}ms", elapsed.count() * 1000);
    }
}
