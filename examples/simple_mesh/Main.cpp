#include <R-Engine/Application.hpp>
#include <R-Engine/Components/Material3d.hpp>
#include <R-Engine/Components/Shader.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

// clang-format off

static inline const auto startup_system_create_shader(const auto &shaders)
{
    const std::string vert_path = r::path::get("examples/simple_mesh/assets/simple.vert");
    const std::string frag_path = r::path::get("examples/simple_mesh/assets/simple.frag");

    r::ShaderHandle shader_handle = shaders.ptr->load(vert_path, frag_path);
    r::Material3d material(shader_handle);
    const ::Shader* shader_ptr = shaders.ptr->get(shader_handle);

    if (shader_ptr) {
        const i32 color_loc = GetShaderLocation(*shader_ptr, "customColor");

        if (color_loc != -1) {
            material.set_uniform_loc("customColor", color_loc);
        }

        material.set_uniform("customColor", r::Vec4f{1.0f, 0.5f, 0.8f, 1.0f});
    }

    return material;
}

static void startup_system(r::ecs::Commands &cmd, r::ecs::ResMut<r::Meshes> meshes, r::ecs::ResMut<r::Shaders> shaders) noexcept
{
    const auto &material = startup_system_create_shader(shaders);

    cmd.spawn(
        r::Mesh3d{
            .id = meshes.ptr->add(r::Mesh3d::Cube(2.0f))
        },
        r::Transform3d{
            .position = {0, 0, 0}
        },
        material
    );
}

i32 main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{})
        .add_systems<startup_system>(r::Schedule::STARTUP)
        .run();
}
