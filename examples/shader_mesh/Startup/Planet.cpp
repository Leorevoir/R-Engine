#include "R-Engine/Plugins/RenderPlugin.hpp"
#include "Startup.hpp"

#include "Startup.hpp"

// clang-format off

static inline const auto startup_system_create_planet_material_uniforms(
    const r::ShaderHandle shader_handle,
    const ::Shader *shader_ptr,
    const f32 global_time
) noexcept
{
    r::Material3d material(shader_handle);

    if (shader_ptr) {
        const i32 color_loc = GetShaderLocation(*shader_ptr, "customColor");

        material.set_uniform_loc("customColor", color_loc);
        material.set_uniform("customColor", r::Vec4f{0.2f, 0.4f, 1.0f, 1.0f});

        const i32 time_loc = GetShaderLocation(*shader_ptr, "globalTime");

        material.set_uniform_loc("globalTime", time_loc);
        material.set_uniform("globalTime", global_time);
    }

    return material;
}

static inline const auto startup_system_create_planet_shader(const auto &shaders, const f32 global_time) noexcept
{
    const std::string vert_path = r::path::get("examples/shader_mesh/assets/planet.vert");
    const std::string frag_path = r::path::get("examples/shader_mesh/assets/planet.frag");

    r::ShaderHandle shader_handle = shaders.ptr->load(vert_path, frag_path);
    const ::Shader *shader_ptr = shaders.ptr->get(shader_handle);

    return startup_system_create_planet_material_uniforms(shader_handle, shader_ptr, global_time);
}

// clang-format off

void r::startup_system_create_planet(
    ecs::Commands &cmd,
    ecs::ResMut<Meshes> meshes,
    ecs::ResMut<Shaders> shaders,
    ecs::Res<core::FrameTime> frame_time 
) noexcept
{
    cmd.spawn(
        Mesh3d{
            .id = meshes.ptr->add(Mesh3d::Sphere(5.0f, 32, 32))
        },
        Transform3d{
            .position = {0, 0, 0}
        },
        startup_system_create_planet_shader(shaders, frame_time.ptr->global_time)
    );
}
