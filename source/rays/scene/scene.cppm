module;

#include "state/options.h"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

export module rays:scene;

import :animation;
import :camera;
import :light;
import :material;
import :matrix;
import :mesh;
import :texture;
import :thread;
import :type;
import :vector;

namespace rays {

/// Scene.
export class Scene {

  public:
    /// Return reference to `Camera`.
    template <typename Self> [[nodiscard]] auto &GetCamera(this Self &&self) {
        return std::forward<Self>(self).camera_;
    }

    /// Return reference to `Texture` vector.
    template <typename Self> [[nodiscard]] auto &GetTextures(this Self &&self) {
        return std::forward<Self>(self).textures_;
    }

    /// Add `Mesh` to scene.
    void AddMesh(const Mesh &&mesh) { meshes_.push_back(std::move(mesh)); }

    /// Add `Light` to scene.
    template <std::derived_from<Light> L> void AddLight(const L &light) {
        lights_.push_back(std::make_unique<L>(light));
    }

    /// Add `Material` to scene.
    void AddMaterial(const Material &&material) {
        materials_.push_back(std::move(material));
    }

    /// Add `Texture` to scene.
    void AddTexture(const Texture &&texture) {
        textures_.push_back(std::move(texture));
    }

    /// Set `Animation` of scene with keyframes sorted by time.
    void SetAnimation(const Animation &animation) {
        Animation sorted = animation;
        const auto SortKeyframes =
            []<typename T>(std::vector<Keyframe<T>> &keyframes) {
                std::ranges::sort(keyframes, {}, &Keyframe<T>::time);
            };
        SortKeyframes(sorted.camera.position);
        SortKeyframes(sorted.camera.matrix);
        animation_ = std::move(sorted);
    }

    /// Return reference to `Animation`.
    template <typename Self>
    [[nodiscard]] auto &GetAnimation(this Self &&self) {
        return std::forward<Self>(self).animation_;
    }

    /// Update scene to given animation frame index.
    void SetFrame(UInt frame) {
        const Animation &animation = animation_;
        if (animation.duration <= 0.0f) {
            return;
        }

        const Float time = static_cast<Float>(frame) / animation.fps;
        if (auto position = SampleKeyframes(animation.camera.position, time)) {
            camera_.SetPosition(*position);
        }
        if (auto rotation = SampleKeyframes(animation.camera.matrix, time)) {
            camera_.SetRotation(*rotation);
        }
    }

    /// Render scene to film.
    void Render(ThreadPool &thread_pool, Rays_Options &options) {
        camera_.Render(thread_pool, meshes_, lights_, materials_, textures_,
                       options);
    }

    /// Preview scene with fast rendering.
    void Preview(ThreadPool &thread_pool, unsigned long long time_budget,
                 Rays_Options &options) {
        camera_.Preview(thread_pool, time_budget, meshes_, lights_, materials_,
                        textures_, options);
    }

  private:
    /// Sample keyframes at given time by linearly interpolating between
    /// bracketing keyframes.  Return `nullopt` when there are no keyframes.
    template <typename T>
    [[nodiscard]] static std::optional<T>
    SampleKeyframes(const std::vector<Keyframe<T>> &keyframes, Float time) {
        if (keyframes.empty()) {
            return std::nullopt;
        }
        if (keyframes.size() == 1 || time <= keyframes.front().time) {
            return keyframes.front().value;
        }
        if (time >= keyframes.back().time) {
            return keyframes.back().value;
        }

        // Find consecutive keyframes bracketing time.
        for (std::size_t i = 0; i + 1 < keyframes.size(); ++i) {
            const Keyframe<T> &a = keyframes[i];
            const Keyframe<T> &b = keyframes[i + 1];
            const Float span = b.time - a.time;
            if (span <= 0.0f || time < a.time || time > b.time) {
                continue;
            }
            const Float t = (time - a.time) / span;
            return a.value * (Float{1} - t) + b.value * t;
        }

        return std::nullopt;
    }

    /// Camera.
    Camera<Float> camera_;
    /// Meshes.
    std::vector<Mesh> meshes_;
    /// Lights.
    std::vector<std::unique_ptr<Light>> lights_;
    /// Materials.
    std::vector<Material> materials_;
    /// Textures.
    std::vector<Texture> textures_;
    /// Animation.
    Animation animation_;
};

} // namespace rays
