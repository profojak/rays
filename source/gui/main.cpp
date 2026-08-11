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
} state;

/// Per-frame input state.
struct Input {
    /// Whether to render.
    bool to_render = false;
    /// Whether camera is currently rendering.
    bool is_rendering = false;
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
    if (state.texture) {
        SDL_DestroyTexture(state.texture);
    }
    if (state.renderer) {
        SDL_DestroyRenderer(state.renderer);
    }
    if (state.window) {
        SDL_DestroyWindow(state.window);
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

    state.window = SDL_CreateWindow("Rays", state.width, state.height,
                                    SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!state.window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        ShutdownSDL();
        return 1;
    }

    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        ShutdownSDL();
        return 1;
    }

    int window_h, window_w, pixel_w, pixel_h;
    SDL_GetWindowSize(state.window, &window_w, &window_h);
    SDL_GetWindowSizeInPixels(state.window, &pixel_w, &pixel_h);
    if (window_w > 0 && window_h > 0) {
        state.scale_x = (float)pixel_w / window_w;
        state.scale_y = (float)pixel_h / window_h;
        SDL_SetRenderScale(state.renderer, state.scale_x, state.scale_y);
    }

    SDL_SetWindowSize(state.window,
                      (int)SDL_ceilf((float)state.width / state.scale_x),
                      (int)SDL_ceilf((float)state.height / state.scale_y));

    state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_RGBA32,
                                      SDL_TEXTUREACCESS_STREAMING, state.width,
                                      state.height);
    if (!state.texture) {
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
    ImGui_ImplSDL3_InitForSDLRenderer(state.window, state.renderer);
    ImGui_ImplSDLRenderer3_Init(state.renderer);
    return 0;
}

/// Render ImGui layout.
void RenderImGUI() {
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos({16, 16}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({240, 80}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Hello, Chaos!");
    ImGui::Text(
        "Keyboard input: %c%c%c%c%c%c", input.move_input.forward ? 'W' : '-',
        input.move_input.left ? 'A' : '-',
        input.move_input.backward ? 'S' : '-',
        input.move_input.right ? 'D' : '-', input.move_input.down ? 'Q' : '-',
        input.move_input.up ? 'E' : '-');
    if (input.is_rendering) {
        ImGui::Text("Rendering...");
    } else {
        if (ImGui::Button("Render")) {
            input.to_render = true;
        }
    }
    ImGui::End();

    ImGui::Render();
}

/// Present one frame.
void RenderFrame() {
    SDL_RenderClear(state.renderer);
    SDL_FRect const viewport = {0, 0, (float)state.width / state.scale_x,
                                (float)state.height / state.scale_y};
    SDL_RenderTexture(state.renderer, state.texture, nullptr, &viewport);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state.renderer);
    SDL_RenderPresent(state.renderer);
}

/// Fetch image data from camera.
void FetchImageData() {
    void *image;
    int pitch;
    if (SDL_LockTexture(state.texture, nullptr, &image, &pitch)) {
        uint8_t *dst = static_cast<uint8_t *>(image);
        uint8_t const *src =
            static_cast<uint8_t const *>(Rays_Camera_ImageData());
        for (int y = 0; y < state.height; ++y) {
            memcpy(dst + y * pitch, src + y * state.width * 4, state.width * 4);
        }
        SDL_UnlockTexture(state.texture);
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

    Rays_Scene_Load(argv[1], Rays_Scene_Type_CRT);
    Rays_Camera_GetResolution(&state.width, &state.height);

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

        if (!input.is_rendering && (Rays_Camera_Move(input.move_input) |
                                    Rays_Camera_Rotate(input.rotate_input))) {
            Uint64 const elapsed = SDL_GetTicks() - frame_start;
            Uint64 const remaining = elapsed < target_frame_time_ms
                                         ? target_frame_time_ms - elapsed
                                         : Uint64{0};
            Rays_Camera_Preview(remaining);
            FetchImageData();
        }

        if (input.to_render) {
            Rays_Camera_Render();
            input.to_render = false;
            input.is_rendering = true;
        } else if (input.is_rendering) {
            FetchImageData();
            if (!Rays_Camera_IsRendering()) {
                input.is_rendering = false;
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
