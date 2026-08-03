extern "C" { // C API
/// Scene format types.
enum Rays_Scene_Type { Rays_Scene_Type_CRT = 0 };

/// Greet folks from Chaos!
void Rays_Greet();
/// Load scene from file.
void Rays_Scene_Load(const char *path, Rays_Scene_Type type);
/// Get camera resolution.
void Rays_Camera_GetResolution(unsigned int *width, unsigned int *height);
/// Get pointer to image data.
const void *Rays_Camera_ImageData();
/// Render scene to film.
void Rays_Camera_Render();
}
