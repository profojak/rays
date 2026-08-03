#include <SDL3/SDL.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

#include "SDL3/SDL_log.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "rays.hpp"

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
} state;

/// Per-frame input state.
struct Input {
    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
} input;

/// Frame duration for 60 FPS.
constexpr static Uint64 target_frame_time_ms = 1000 / 60;

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

    state.window = SDL_CreateWindow("Rays", state.width, state.height, 0);
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
    ImGui::Begin("Chaos");
    ImGui::TextUnformatted("Hello, Chaos!");
    ImGui::Text("Keyboard input: %c%c%c%c%c%c", input.forward ? 'W' : '-',
                input.left ? 'A' : '-', input.back ? 'S' : '-',
                input.right ? 'D' : '-', input.down ? 'Q' : '-',
                input.up ? 'E' : '-');
    ImGui::End();

    ImGui::Render();
}

/// Present one frame.
void RenderFrame() {
    SDL_RenderClear(state.renderer);
    SDL_FRect const viewport = {0, 0, (float)state.width, (float)state.height};
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

    bool const *keys = SDL_GetKeyboardState(nullptr);

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

        input.forward = keys[SDL_SCANCODE_W];
        input.back = keys[SDL_SCANCODE_S];
        input.left = keys[SDL_SCANCODE_A];
        input.right = keys[SDL_SCANCODE_D];
        input.up = keys[SDL_SCANCODE_E];
        input.down = keys[SDL_SCANCODE_Q];

        if (input.forward) {
            Rays_Camera_Render();
        }

        FetchImageData();

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
