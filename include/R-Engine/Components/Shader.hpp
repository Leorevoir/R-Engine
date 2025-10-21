#pragma once

#include <R-Engine/Components/Material3d.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <string>
#include <vector>

namespace r {

class Shaders final
{
    public:
        Shaders() = default;
        ~Shaders()
        {
            for (const auto &shader : _shaders) {
                if (shader.id > 0) {
                    UnloadShader(shader);
                }
            }
        }

        // Load a shader from vertex and fragment shader files
        ShaderHandle load(const std::string &vs_path, const std::string &fs_path)
        {
            if (!path::exists(vs_path) || !path::exists(fs_path)) {
                Logger::error("Shader file not found.");
                return ShaderInvalidHandle;
            }

            ::Shader shader = LoadShader(vs_path.c_str(), fs_path.c_str());
            if (shader.id <= 0) {
                Logger::error("Failed to load shader.");
                return ShaderInvalidHandle;
            }

            _shaders.push_back(shader);
            return static_cast<ShaderHandle>(_shaders.size() - 1);
        }

        // Get a loaded shader by its handle
        const ::Shader *get(ShaderHandle handle) const
        {
            if (handle >= _shaders.size()) {
                return nullptr;
            }
            return &_shaders[handle];
        }

    private:
        std::vector<::Shader> _shaders;
};

}// namespace r
