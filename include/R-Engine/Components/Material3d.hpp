#pragma once

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Types.hpp>

namespace r {

using ShaderHandle = u32;
static constexpr inline const ShaderHandle ShaderInvalidHandle = static_cast<ShaderHandle>(-1);

struct R_ENGINE_API Material3d {
        ShaderHandle shader_id = ShaderInvalidHandle;
};

}// namespace r
