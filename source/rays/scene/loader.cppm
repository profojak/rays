module;

#include <rapidjson/document.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

export module rays:loader;

import :camera;
import :scene;
import :type;
import :vector;

namespace rays {

/// Base class for scene loaders.
export class Loader {

  public:
    /// Type of scene to load.
    enum class Type : std::uint8_t {
        CRT = 0,
    };

    /// Load scene from given file path.
    static std::unique_ptr<Scene> Load(const std::string &path) {
        std::unreachable();
    }
};

/// Loader for CRT scenes.
export class CRTLoader : public Loader {

  public:
    /// Load scene from given .crtscene file.
    [[nodiscard]] static std::unique_ptr<Scene> Load(const std::string &path) {
        std::unique_ptr<Scene> scene = std::make_unique<Scene>();

        // Open scene file and parse as JSON.
        std::ifstream file{path};
        if (!file) {
            throw std::runtime_error{"Failed to open scene file '" + path +
                                     "'!"};
        }
        const std::string json{std::istreambuf_iterator<char>{file},
                               std::istreambuf_iterator<char>{}};
        rapidjson::Document document;
        if (document.Parse(json.c_str()).HasParseError()) {
            throw std::runtime_error{"Failed to parse scene file '" + path +
                                     "'!"};
        }
        if (!document.IsObject()) {
            throw std::runtime_error{"Scene file '" + path +
                                     "' is not valid JSON object!"};
        }

        // Settings.
        if (const auto &settings = document["settings"]; settings.IsObject()) {
            // Background color.
            if (const auto &color = settings["background_color"];
                color.IsArray() && color.Size() == 3) {
                [[maybe_unused]] const Vector3f background_color{
                    color[0].GetFloat(), color[1].GetFloat(),
                    color[2].GetFloat()};
                scene->GetCamera().GetFilm().SetBackground(background_color);
            }

            // Image settings.
            if (const auto &image = settings["image_settings"];
                image.IsObject()) {
                const auto &width = image["width"];
                const auto &height = image["height"];
                if (width.IsUint() && height.IsUint()) {
                    scene->GetCamera().SetFilmSize(
                        Vector2u{width.GetUint(), height.GetUint()});
                }
            }
        }

        // Camera.
        if (const auto &camera = document["camera"]; camera.IsObject()) {
            // Rotation matrix.
            if (const auto &matrix = camera["matrix"];
                matrix.IsArray() && matrix.Size() == 9) {
                std::array<Float, 9> rotation_matrix{};
                for (std::size_t i = 0; i < 9; ++i) {
                    rotation_matrix[i] = matrix[i].GetFloat();
                }
                scene->GetCamera().SetRotation(Matrix3f{rotation_matrix});
            }

            // Camera position.
            if (const auto &position_value = camera["position"];
                position_value.IsArray() && position_value.Size() == 3) {
                [[maybe_unused]] const Vector3f position{
                    position_value[0].GetFloat(), position_value[1].GetFloat(),
                    position_value[2].GetFloat()};
                scene->GetCamera().SetPosition(position);
            }
        }

        // Objects.
        if (const auto &objects = document["objects"]; objects.IsArray()) {
            for (const auto &object : objects.GetArray()) {
                if (!object.IsObject()) {
                    continue;
                }

                // Vertices.
                if (const auto &vertices = object["vertices"];
                    vertices.IsArray() && vertices.Size() % 3 == 0) {
                    std::vector<Vector3f> positions;
                    positions.reserve(vertices.Size() / 3);
                    for (std::size_t i = 0; i < vertices.Size(); i += 3) {
                        positions.push_back(Vector3f{
                            vertices[i].GetFloat(), vertices[i + 1].GetFloat(),
                            vertices[i + 2].GetFloat()});
                    }
                    // TODO
                }

                // Triangles.
                if (const auto &triangles = object["triangles"];
                    triangles.IsArray() && triangles.Size() % 3 == 0) {
                    std::vector<UInt> indices;
                    indices.reserve(triangles.Size());
                    for (std::size_t i = 0; i < triangles.Size(); ++i) {
                        indices.push_back(triangles[i].GetUint());
                    }
                    // TODO
                }
            }
        }

        return scene;
    }
};

} // namespace rays
