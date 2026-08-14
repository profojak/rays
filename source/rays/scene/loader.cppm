module;

#include <rapidjson/document.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

export module rays:loader;

import :camera;
import :light;
import :material;
import :mesh;
import :scene;
import :texture;
import :triangle;
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
        throw std::logic_error{"`Loader::Load()` is not implemented!"};
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
                scene->GetCamera().SetRotation(
                    Matrix3f{rotation_matrix}.Transposed());
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

        // Lights.
        if (const auto &lights = document["lights"]; lights.IsArray()) {
            for (const auto &light : lights.GetArray()) {
                if (!light.IsObject()) {
                    continue;
                }

                Float intensity = 0.0f;
                Vector3f position{0.0f};

                // Light intensity.
                if (const auto &intensity_value = light["intensity"];
                    intensity_value.IsNumber()) {
                    intensity = intensity_value.GetFloat();
                }

                // Light position.
                if (const auto &position_value = light["position"];
                    position_value.IsArray() && position_value.Size() == 3) {
                    position = Vector3f{position_value[0].GetFloat(),
                                        position_value[1].GetFloat(),
                                        position_value[2].GetFloat()};
                }

                scene->AddLight(PointLight{intensity, position});
            }
        }

        // Textures.
        if (const auto &textures = document["textures"]; textures.IsArray()) {
            for (const auto &texture : textures.GetArray()) {
                if (!texture.IsObject()) {
                    continue;
                }

                std::string name;
                Texture::Type type;
                Vector3f albedo{1.0f};

                // Name.
                if (const auto &name_value = texture["name"];
                    name_value.IsString()) {
                    name = name_value.GetString();
                } else {
                    throw std::runtime_error{"Texture `name` field is "
                                             "missing or invalid in scene `" +
                                             path + "`!"};
                }

                // Type.
                if (const auto &type_value = texture["type"];
                    type_value.IsString()) {
                    const std::string type_str = type_value.GetString();
                    if (type_str == "albedo") {
                        type = Texture::Type::Albedo;
                    } else {
                        /*
                        throw std::runtime_error{"Unknown texture type `" +
                                                 type_str + "` in scene `" +
                                                 path + "`!"};
                                                 */
                    }
                } else {
                    throw std::runtime_error{"Texture `type` field is "
                                             "missing or invalid in scene `" +
                                             path + "`!"};
                }

                // Albedo.
                if (const auto &albedo_value = texture["albedo"];
                    albedo_value.IsArray() && albedo_value.Size() == 3) {
                    albedo = {albedo_value[0].GetFloat(),
                              albedo_value[1].GetFloat(),
                              albedo_value[2].GetFloat()};
                }

                scene->AddTexture(Texture{std::move(name), type, albedo});
            }
        }

        // Materials.
        if (const auto &materials = document["materials"];
            materials.IsArray()) {
            for (const auto &material : materials.GetArray()) {
                if (!material.IsObject()) {
                    continue;
                }

                Material::Type type;
                std::variant<Vector3f, UInt> albedo{Vector3f{1.0f}};
                bool smooth_shading;
                Float index_of_refraction = 1.5f;

                // Type.
                if (const auto &type_value = material["type"];
                    type_value.IsString()) {
                    const std::string type_str = type_value.GetString();
                    if (type_str == "constant") {
                        type = Material::Type::Constant;
                    } else if (type_str == "reflective") {
                        type = Material::Type::Reflective;
                    } else if (type_str == "refractive") {
                        type = Material::Type::Refractive;
                    } else if (type_str == "diffuse") {
                        type = Material::Type::Diffuse;
                    } else {
                        throw std::runtime_error{"Unknown material type `" +
                                                 type_str + "` in scene `" +
                                                 path + "`!"};
                    }
                } else {
                    throw std::runtime_error{
                        "Material `type` field is missing or invalid in "
                        "scene `" +
                        path + "`!"};
                }

                // Albedo.
                if (const auto &albedo_value = material["albedo"];
                    albedo_value.IsArray() && albedo_value.Size() == 3) {
                    albedo = Vector3f{albedo_value[0].GetFloat(),
                                      albedo_value[1].GetFloat(),
                                      albedo_value[2].GetFloat()};
                } else if (albedo_value.IsString()) {
                    const std::string_view albedo_texture =
                        albedo_value.GetString();
                    const auto &textures = scene->GetTextures();
                    const auto it = std::ranges::find_if(
                        textures, [&](const Texture &texture) {
                            return texture.name == albedo_texture;
                        });
                    if (it == textures.end()) {
                        throw std::runtime_error{"Unknown albedo texture `" +
                                                 std::string{albedo_texture} +
                                                 "` in scene `" + path + "`!"};
                    }
                    albedo =
                        static_cast<UInt>(std::distance(textures.begin(), it));
                }

                // Smooth shading.
                if (const auto &smooth_shading_value =
                        material["smooth_shading"];
                    smooth_shading_value.IsBool()) {
                    smooth_shading = smooth_shading_value.GetBool();
                } else {
                    throw std::runtime_error{
                        "Material `smooth_shading` field is missing or "
                        "invalid in scene `" +
                        path + "`!"};
                }

                // Index of refraction for refractive materials.
                if (const auto &ior_value = material["ior"];
                    ior_value.IsNumber()) {
                    index_of_refraction = ior_value.GetFloat();
                }

                scene->AddMaterial(std::move(Material{
                    type, albedo, smooth_shading, index_of_refraction}));
            }
        }

        // Objects.
        if (const auto &objects = document["objects"]; objects.IsArray()) {
            for (const auto &object : objects.GetArray()) {
                if (!object.IsObject()) {
                    continue;
                }

                Mesh mesh;

                // Material index.
                if (const auto &material_index = object["material_index"];
                    material_index.IsInt()) {
                    mesh.material_index = material_index.GetInt();
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
                    mesh.vertices = std::move(positions);
                }

                // Texture coordinates.
                if (const auto &uvs = object["uvs"];
                    uvs.IsArray() && uvs.Size() % 3 == 0) {
                    std::vector<Vector3f> tex_coords;
                    tex_coords.reserve(uvs.Size() / 3);
                    for (std::size_t i = 0; i < uvs.Size(); i += 3) {
                        tex_coords.push_back(Vector3f{uvs[i].GetFloat(),
                                                      uvs[i + 1].GetFloat(),
                                                      uvs[i + 2].GetFloat()});
                    }
                    mesh.uvs = std::move(tex_coords);
                }

                // Triangles.
                if (const auto &triangles = object["triangles"];
                    triangles.IsArray() && triangles.Size() % 3 == 0) {
                    std::vector<Triangle> indices;
                    indices.reserve(triangles.Size() / 3);
                    for (std::size_t i = 0; i < triangles.Size(); i += 3) {
                        indices.push_back(Triangle{triangles[i].GetUint(),
                                                   triangles[i + 1].GetUint(),
                                                   triangles[i + 2].GetUint()});
                    }
                    mesh.triangles = std::move(indices);
                }

                mesh.RecalculateNormals();
                scene->AddMesh(std::move(mesh));
            }
        }

        return scene;
    }
};

} // namespace rays
