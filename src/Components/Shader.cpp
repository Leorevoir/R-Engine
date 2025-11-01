#include <R-Engine/Components/Shader.hpp>
#include <R-Engine/Maths/Vec.hpp>

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
    if (!path::exists(vs_path)) {
        Logger::error("Shader file not found: " + vs_path);
        return ShaderInvalidHandle;
    }

    if (!path::exists(fs_path)) {
        Logger::error("Shader file not found: " + fs_path);
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

void r::Shaders::set_value(const ::Shader &shader, const r::ShaderLocation loc, const std::any &data_any) noexcept
{
    if (loc == r::ShaderInvalidLocation) {
        return;
    }
    if (data_any.type() == typeid(i32)) {
        const i32 value = std::any_cast<i32>(data_any);
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_INT);
    } else if (data_any.type() == typeid(f32)) {
        const f32 value = std::any_cast<f32>(data_any);
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_FLOAT);
    } else if (data_any.type() == typeid(Vec2f)) {
        const Vec2f value = std::any_cast<Vec2f>(data_any);
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_VEC2);
    } else if (data_any.type() == typeid(Vec3f)) {
        const Vec3f value = std::any_cast<Vec3f>(data_any);
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_VEC3);
    } else if (data_any.type() == typeid(Vec4f)) {
        const Vec4f value = std::any_cast<Vec4f>(data_any);
        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_VEC4);
    }
}
