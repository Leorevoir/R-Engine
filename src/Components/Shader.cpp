#include <R-Engine/Components/Shader.hpp>

/**
* public
*/

r::Shaders::~Shaders() noexcept
{
    for (const auto &shader : _shaders) {
        if (shader.id > 0) {
            UnloadShader(shader);
        }
    }
}

r::ShaderHandle r::Shaders::load(const std::string &vs_path, const std::string &fs_path)
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

const ::Shader *r::Shaders::get(r::ShaderHandle handle) const
{
    if (handle >= _shaders.size()) {
        return nullptr;
    }
    return &_shaders[handle];
}
