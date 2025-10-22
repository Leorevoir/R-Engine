#pragma once

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Types.hpp>
#include <any>
#include <string>
#include <unordered_map>

namespace r {

using ShaderHandle = u32;
using ShaderLocation = i32;
static constexpr inline const ShaderHandle ShaderInvalidHandle = static_cast<ShaderHandle>(-1);
static constexpr inline const ShaderLocation ShaderInvalidLocation = -1;

/**
* @brief Material3d class represents a 3D material with shader and uniform management.
*/
class R_ENGINE_API Material3d
{
    public:
        Material3d() noexcept = default;
        Material3d(const ShaderHandle shader) noexcept;
        ~Material3d() noexcept = default;

        void set_shader(ShaderHandle shader) noexcept;
        void set_uniform(const std::string &name, const std::any &value) noexcept;
        void set_uniform_loc(const std::string &name, const ShaderLocation location) noexcept;

        ShaderHandle get_shader() const noexcept;
        const std::unordered_map<std::string, std::any> &get_uniforms() const noexcept;
        ShaderLocation get_uniform_loc(const std::string &name) const noexcept;

        template<typename T>
        T get_uniform(const std::string &name) const;

    private:
        ShaderHandle _shader_id = ShaderInvalidHandle;
        std::unordered_map<std::string, std::any> _uniforms;
        std::unordered_map<std::string, ShaderLocation> _uniform_locations;
};

}// namespace r

#include "Inline/Material3d.inl"
