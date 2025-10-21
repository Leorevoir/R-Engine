#include <R-Engine/Application.hpp>
#include <R-Engine/Components/Material3d.hpp>
#include <R-Engine/Components/Shader.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

static void startup_system(r::ecs::Commands &cmd, r::ecs::ResMut<r::Meshes> meshes, r::ecs::ResMut<r::Shaders> shaders)
{
    const std::string vs_path = r::path::get("examples/simple_mesh/assets/simple.vert");
    const std::string fs_path = r::path::get("examples/simple_mesh/assets/simple.frag");
    const r::ShaderHandle custom_shader_handle = shaders.ptr->load(vs_path, fs_path);
    const r::MeshHandle cube_handle = meshes.ptr->add(r::Mesh3d::Cube(2.0f));

    cmd.spawn(r::Mesh3d{.id = cube_handle}, r::Transform3d{.position = {0, 0, 0}}, r::Material3d{.shader_id = custom_shader_handle});
}

int main(void)
{
    r::Application{}.add_plugins(r::DefaultPlugins{}).add_systems<startup_system>(r::Schedule::STARTUP).run();
}
