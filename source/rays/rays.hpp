extern "C" { // C API
/// Scene format types.
enum Rays_Scene_Type { Rays_Scene_Type_CRT = 0 };
/// Camera input state.
struct Rays_Camera_MoveInput {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
};

/// Greet folks from Chaos!
void Rays_Greet();
/// Load scene from file.
void Rays_Scene_Load(const char *path, Rays_Scene_Type type);
/// Move camera based on input.
bool Rays_Camera_Move(const Rays_Camera_MoveInput &input);
/// Get camera resolution.
void Rays_Camera_GetResolution(unsigned int *width, unsigned int *height);
/// Get pointer to image data.
const void *Rays_Camera_ImageData();
/// Render scene to film.
void Rays_Camera_Render();
}
