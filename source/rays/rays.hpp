extern "C" { // C API
/// Greet folks from Chaos!
void Rays_Greet();
/// Create camera with given film size.
void Rays_Camera_Create(unsigned int width, unsigned int height);
/// Get pointer to image data.
const void *Rays_Camera_ImageData();
}
