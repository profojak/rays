#include <chrono>
#include <filesystem>
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

    Rays_Options options{.sample_all_lights = true, .samples_per_pixel = 1};

    auto start = std::chrono::high_resolution_clock::now();
    std::println("Rendering...");

    rays::Render(options);
    rays::WaitForRender();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::println("Rendered in {:.2f}ms", elapsed.count() * 1000);

    rays::SaveImage(std::filesystem::current_path().string());
}
