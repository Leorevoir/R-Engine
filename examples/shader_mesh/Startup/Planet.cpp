#include "Startup.hpp"

#include "Startup.hpp"

static inline const auto startup_system_create_planet_material(const auto &shaders)
{
    const std::string vert_path = r::path::get("examples/shader_mesh/assets/simple.vert");
    const std::string frag_path = r::path::get("examples/shader_mesh/assets/simple.frag");

    r::ShaderHandle shader_handle = shaders.ptr->load(vert_path, frag_path);
    r::Material3d material(shader_handle);
    const ::Shader *shader_ptr = shaders.ptr->get(shader_handle);

    if (shader_ptr) {
        const i32 color_loc = GetShaderLocation(*shader_ptr, "customColor");

        if (color_loc != -1) {
            material.set_uniform_loc("customColor", color_loc);
        }

        material.set_uniform("customColor", r::Vec4f{0.2f, 0.4f, 1.0f, 1.0f});
    }

    return material;
}

// clang-format off

void r::startup_system_create_planet(ecs::Commands &cmd, ecs::ResMut<Meshes> meshes, ecs::ResMut<Shaders> shaders) noexcept
{
    cmd.spawn(
        Mesh3d{
            .id = meshes.ptr->add(Mesh3d::Sphere(5.0f, 32, 32))
        },
        Transform3d{
            .position = {0, 0, 0}
        },
        startup_system_create_planet_material(shaders)
    );
}
