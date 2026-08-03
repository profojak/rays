module;

#include <rapidjson/document.h>

#include <cstddef>
#include <string>

export module rays:loader;

import :camera;
import :scene;
import :type;
import :vector;

namespace rays {

/// Abstract base class for scene loaders.
export class Loader {

  public:
    virtual ~Loader() = default;

    /// Load scene from given file path.
    virtual Scene Load(const std::string &path) = 0;
};

/// Loader for CRT scenes.
export class CRTLoader : public Loader {

  public:
    /// Load scene from given .crtscene file.
    [[nodiscard]] Scene Load(const std::string &path) override {
        Scene scene{};

        return scene;
    }
};

} // namespace rays
