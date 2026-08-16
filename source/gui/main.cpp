#include <algorithm>
#include <cmath>

#include <filesystem>
#include <functional>

#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "rays.h"

/// Global state for SDL3.
struct SDL_State {
    /// Window handle.
    SDL_Window *window = nullptr;
    /// Renderer handle.
    SDL_Renderer *renderer = nullptr;
    /// Texture handle.
    SDL_Texture *texture = nullptr;
    /// Window width.
    inline static unsigned int width = 1280;
    /// Window height.
    inline static unsigned int height = 720;
    /// Pixels per window width.
    inline static float scale_x = 1.0f;
    /// Pixels per window height.
    inline static float scale_y = 1.0f;
} sdl_state;

/// Global state of renderer.
struct Rays_State {
    /// Whether to render.
    bool to_render = false;
    /// Whether camera is currently rendering.
    bool is_rendering = false;
    /// Whether render is completed.
    bool is_rendered = false;
    /// Animation duration in seconds.
    float animation_duration = 0.0f;
    /// Animation frames per second.
    float animation_fps = 0.0f;
    /// Current selected animation time in seconds.
    float animation_time = 0.0f;
    /// Callback invoked whenever animation slider changes.
    std::function<void(float)> on_animation_time_changed;
    /// Rendering options.
    Rays_Options options;
} rays_state;

/// Per-frame input state.
struct Input {
    /// Camera move input.
    Rays_Camera_MoveInput move_input;
    /// Camera rotate input.
    Rays_Camera_RotateInput rotate_input;
    /// Whether left mouse button is dragging.
    bool dragging = false;
    /// Last mouse X position.
    float last_mouse_x = 0.0f;
    /// Last mouse Y position.
    float last_mouse_y = 0.0f;
} input;

/// Frame duration for 60 FPS.
constexpr static Uint64 target_frame_time_ms = 1000 / 60;

/// Camera rotation sensitivity in radians per pixel.
constexpr static float rotation_sensitivity = 0.003f;

/// Shutdown SDL.
void ShutdownSDL() {
    if (sdl_state.texture) {
        SDL_DestroyTexture(sdl_state.texture);
    }
    if (sdl_state.renderer) {
        SDL_DestroyRenderer(sdl_state.renderer);
    }
    if (sdl_state.window) {
        SDL_DestroyWindow(sdl_state.window);
    }
    SDL_Quit();
}

/// Shutdown ImGui.
void ShutdownImGUI() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

/// Initialize SDL3.
int InitializeSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    sdl_state.window =
        SDL_CreateWindow("Rays", sdl_state.width, sdl_state.height,
                         SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!sdl_state.window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        ShutdownSDL();
        return 1;
    }

    sdl_state.renderer = SDL_CreateRenderer(sdl_state.window, nullptr);
    if (!sdl_state.renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        ShutdownSDL();
        return 1;
    }

    int window_h, window_w, pixel_w, pixel_h;
    SDL_GetWindowSize(sdl_state.window, &window_w, &window_h);
    SDL_GetWindowSizeInPixels(sdl_state.window, &pixel_w, &pixel_h);
    if (window_w > 0 && window_h > 0) {
        sdl_state.scale_x = (float)pixel_w / window_w;
        sdl_state.scale_y = (float)pixel_h / window_h;
        SDL_SetRenderScale(sdl_state.renderer, sdl_state.scale_x,
                           sdl_state.scale_y);
    }

    SDL_SetWindowSize(
        sdl_state.window,
        (int)SDL_ceilf((float)sdl_state.width / sdl_state.scale_x),
        (int)SDL_ceilf((float)sdl_state.height / sdl_state.scale_y));

    sdl_state.texture = SDL_CreateTexture(
        sdl_state.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        sdl_state.width, sdl_state.height);
    if (!sdl_state.texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        ShutdownSDL();
        return 1;
    }

    return 0;
}

/// Initialize ImGui.
int InitializeImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui_ImplSDL3_InitForSDLRenderer(sdl_state.window, sdl_state.renderer);
    ImGui_ImplSDLRenderer3_Init(sdl_state.renderer);
    return 0;
}

/// Render ImGui layout.
void RenderImGUI() {
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();

    // Keyboard input.
    ImGui::SetNextWindowPos({16, 16}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGui::Begin("Hello, Chaos!");
    ImGui::Text(
        "Keyboard input: %c%c%c%c%c%c", input.move_input.forward ? 'W' : '-',
        input.move_input.left ? 'A' : '-',
        input.move_input.backward ? 'S' : '-',
        input.move_input.right ? 'D' : '-', input.move_input.down ? 'Q' : '-',
        input.move_input.up ? 'E' : '-');

    // Render & save button.
    ImGui::BeginDisabled(rays_state.is_rendering);
    if (ImGui::Button(rays_state.is_rendering ? "Rendering" : "Render")) {
        rays_state.to_render = true;
    }
    ImGui::SetItemTooltip("Render the scene from the current view.");
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!rays_state.is_rendered || rays_state.is_rendering);
    if (ImGui::Button("Save .ppm")) {
        Rays_SaveImage(std::filesystem::current_path().string().c_str());
    }
    ImGui::SetItemTooltip(
        "Save the render as an output.ppm file to the current directory.");
    ImGui::EndDisabled();
    ImGui::Separator();

    // Rendering options.
    ImGui::Checkbox("Sample all lights", &rays_state.options.sample_all_lights);
    ImGui::SetItemTooltip("Sample all lights in the scene on each ray hit, or "
                          "randomly pick one of the lights.");
    ImGui::Text("Samples per pixel:");
    ImGui::SetNextItemWidth(128);
    ImGui::SliderInt("##a", &rays_state.options.samples_per_pixel, 1, 32);
    ImGui::SetItemTooltip("How many rays to cast per pixel.");
    ImGui::Separator();
    ImGui::Checkbox("Use two-level SBVH", &rays_state.options.use_bvh);
    ImGui::SetItemTooltip("Use two-level bounding volume hierarchy to "
                          "accelerate ray-triangle intersections.");
    ImGui::Checkbox("Global illumination",
                    &rays_state.options.global_illumination);
    ImGui::SetItemTooltip(
        "Whether to illuminate globally, or only compute direct lighting.");

    // Animation.
    ImGui::Separator();
    ImGui::Text("Animation time:");
    ImGui::SetNextItemWidth(128);
    if (ImGui::SliderFloat("##b", &rays_state.animation_time, 0.0f,
                           rays_state.animation_duration)) {
        if (rays_state.on_animation_time_changed) {
            rays_state.on_animation_time_changed(rays_state.animation_time);
        }
    }
    ImGui::SetItemTooltip("Select animation frame to preview at a given time.");

    ImGui::End();
    ImGui::Render();
}

/// Present one frame.
void RenderFrame() {
    SDL_RenderClear(sdl_state.renderer);
    SDL_FRect const viewport = {0, 0,
                                (float)sdl_state.width / sdl_state.scale_x,
                                (float)sdl_state.height / sdl_state.scale_y};
    SDL_RenderTexture(sdl_state.renderer, sdl_state.texture, nullptr,
                      &viewport);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                          sdl_state.renderer);
    SDL_RenderPresent(sdl_state.renderer);
}

/// Fetch image data from camera.
void FetchImageData() {
    void *image;
    int pitch;
    if (SDL_LockTexture(sdl_state.texture, nullptr, &image, &pitch)) {
        uint8_t *dst = static_cast<uint8_t *>(image);
        uint8_t const *src = static_cast<uint8_t const *>(Rays_ImageData());
        for (int y = 0; y < sdl_state.height; ++y) {
            memcpy(dst + y * pitch, src + y * sdl_state.width * 4,
                   sdl_state.width * 4);
        }
        SDL_UnlockTexture(sdl_state.texture);
    }
}

/// Process input events.
void ProcessInput() {
    bool const *keys = SDL_GetKeyboardState(nullptr);
    input.move_input.forward = keys[SDL_SCANCODE_W];
    input.move_input.backward = keys[SDL_SCANCODE_S];
    input.move_input.left = keys[SDL_SCANCODE_A];
    input.move_input.right = keys[SDL_SCANCODE_D];
    input.move_input.up = keys[SDL_SCANCODE_E];
    input.move_input.down = keys[SDL_SCANCODE_Q];

    float mouse_x, mouse_y;
    Uint32 const buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
    input.dragging =
        (buttons & SDL_BUTTON_LMASK) != 0 && !ImGui::GetIO().WantCaptureMouse;
    input.rotate_input.yaw =
        input.dragging ? (mouse_x - input.last_mouse_x) * rotation_sensitivity
                       : 0.0f;
    input.rotate_input.pitch =
        input.dragging ? -(mouse_y - input.last_mouse_y) * rotation_sensitivity
                       : 0.0f;
    input.last_mouse_x = mouse_x;
    input.last_mouse_y = mouse_y;
}

int main(int argc, char **argv) {
    Rays_Greet();

    if (argc < 2) {
        printf("Usage: %s <scene>\n", argv[0]);
        return 1;
    }

    Rays_LoadScene(argv[1], Rays_Scene_Type_CRT);
    Rays_GetResolution(&sdl_state.width, &sdl_state.height);

    rays_state.animation_duration = Rays_AnimationDuration();
    rays_state.animation_fps = Rays_AnimationFPS();
    if (rays_state.animation_duration > 0.0f) {
        rays_state.on_animation_time_changed = [](const float time) {
            const unsigned int frame_count =
                std::max(1u, static_cast<unsigned int>(
                                 std::lround(rays_state.animation_duration *
                                             rays_state.animation_fps)));
            unsigned int frame =
                std::clamp(static_cast<unsigned int>(
                               std::lround(time * rays_state.animation_fps)),
                           0u, frame_count - 1);
            Rays_PreviewAnimationFrame(frame, target_frame_time_ms,
                                       rays_state.options);
        };
    }

    if (InitializeSDL()) {
        return 1;
    }

    if (InitializeImGUI()) {
        return 1;
    }

    bool running = true;
    while (running) {
        Uint64 const frame_start = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        ProcessInput();

        if (!rays_state.is_rendering &&
            (Rays_MoveCamera(input.move_input) |
             Rays_RotateCamera(input.rotate_input))) {
            rays_state.is_rendered = false;
            Uint64 const elapsed = SDL_GetTicks() - frame_start;
            Uint64 const remaining = elapsed < target_frame_time_ms
                                         ? target_frame_time_ms - elapsed
                                         : Uint64{0};
            Rays_Preview(remaining, rays_state.options);
            FetchImageData();
        }

        if (rays_state.to_render) {
            Rays_Render(rays_state.options);
            rays_state.to_render = false;
            rays_state.is_rendering = true;
        } else if (rays_state.is_rendering) {
            FetchImageData();
            if (!Rays_IsRendering()) {
                rays_state.is_rendering = false;
                rays_state.is_rendered = true;
            }
        }

        RenderImGUI();
        RenderFrame();

        Uint64 const elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < target_frame_time_ms) {
            SDL_Delay(target_frame_time_ms - elapsed);
        }
    }

    ShutdownImGUI();
    ShutdownSDL();
    return 0;
}
