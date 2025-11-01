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
        Shaders() noexcept = default;
        ~Shaders() noexcept;

        ShaderHandle load(const std::string &vs_path, const std::string &fs_path);

        const ::Shader *get(ShaderHandle handle) const;

        static void set_value(const ::Shader &shader, const r::ShaderLocation loc, const std::any &data_any) noexcept;

    private:
        std::vector<::Shader> _shaders;
};

}// namespace r
