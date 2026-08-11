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
/// Camera rotation input state in radians.
struct Rays_Camera_RotateInput {
    /// Yaw delta, positive turns right.
    float yaw = 0.0f;
    /// Pitch delta, positive looks up.
    float pitch = 0.0f;
};

/// Greet folks from Chaos!
void Rays_Greet();
/// Load scene from file.
void Rays_Scene_Load(const char *path, Rays_Scene_Type type);
/// Move camera relative to view direction.
bool Rays_Camera_Move(const Rays_Camera_MoveInput &input);
/// Rotate camera based on yaw and pitch deltas in radians.
bool Rays_Camera_Rotate(const Rays_Camera_RotateInput &input);
/// Get camera resolution.
void Rays_Camera_GetResolution(unsigned int *width, unsigned int *height);
/// Get pointer to image data.
const void *Rays_Camera_ImageData();
/// Render scene to film.
void Rays_Camera_Render();
/// Check if camera render is in progress.
bool Rays_Camera_IsRendering();
/// Block until camera render finishes.
void Rays_Camera_WaitForRender();
/// Preview scene with fast rendering.
void Rays_Camera_Preview(unsigned long long time_budget);
}
