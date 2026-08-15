#ifndef RAYS_OPTIONS_H
#define RAYS_OPTIONS_H

/// Options for renderer.
struct Rays_Options {
    /// Whether to sample all lights, or randomly pick one.
    bool sample_all_lights = true;
    /// Whether to use two-level SBVH to accelerate ray-triangle
    /// intersections.
    bool use_bvh = true;
    /// Number of samples per pixel.
    int samples_per_pixel = 1;
};

#endif // RAYS_OPTIONS_H
