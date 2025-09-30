#pragma once

#include <R-Engine/Maths/Vec.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/R-EngineExport.hpp>

namespace r {

// clang-format off

enum class Projection : i32 {
    PERSPECTIVE = 0,
    ORTHOGRAPHIC = 1
};

struct R_ENGINE_API Camera3d {
    public:
        Vec3f position          = {.0f, 2.f, 6.f};
        Vec3f target            = {.0f, .0f, .0f};
        Vec3f up                = {.0f, 1.f , .0f};
        f32 fovy                = 45.f;
        Projection projection   = Projection::PERSPECTIVE;
};

// clang-format on

class R_ENGINE_API RenderPlugin final : public Plugin
{
    public:
        RenderPlugin(const r::Camera3d &camera = Camera3d());
        void build(Application &app) override;

    private:
        Camera3d _camera;
};

}// namespace r
