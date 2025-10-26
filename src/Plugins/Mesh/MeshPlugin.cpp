#include <R-Engine/Components/Material3d.hpp>
#include <R-Engine/Components/Shader.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Maths/Quaternion.hpp>

/**
 * static helpers
 */

namespace {

/**
 * Query
 */

// clang-format off

using MeshRenderQuery = r::ecs::Query<
    r::ecs::Ref<r::Mesh3d>,
    r::ecs::Ref<r::GlobalTransform3d>,
    r::ecs::Optional<r::Material3d>
>;

/**
 * Helpers
 */

static inline void mesh_plugin_set_shader_value(const ::Shader &shader, const r::ShaderLocation loc, const std::any &data_any) noexcept
{
    if (data_any.type() == typeid(r::Vec4f)) {
        const auto value = std::any_cast<r::Vec4f>(data_any);

        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_VEC4);
    } else if (data_any.type() == typeid(f32)) {
        const auto value = std::any_cast<f32>(data_any);

        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_FLOAT);
    } else if (data_any.type() == typeid(i32)) {
        const auto value = std::any_cast<i32>(data_any);

        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_INT);
    } else if (data_any.type() == typeid(r::Vec3f)) {
        const auto value = std::any_cast<r::Vec3f>(data_any);

        SetShaderValue(shader, loc, &value, SHADER_UNIFORM_VEC3);
    }
}

static inline void mesh_plugin_send_uniforms(const ::Shader &shader, const r::Material3d &material) noexcept
{
    for (const auto &[name, data_any] : material.get_uniforms()) {
        const r::ShaderLocation loc = material.get_uniform_loc(name);

        if (loc == r::ShaderInvalidLocation) {
            continue;
        }

        mesh_plugin_set_shader_value(shader, loc, data_any);
    }
}

static inline const r::Transform3d mesh_plugin_get_transform3d(
    const r::GlobalTransform3d &global_transform,
    const r::Mesh3d &mesh_component
) noexcept
{
    const r::Quaternion logical_rot = r::Quaternion::from_euler(global_transform.rotation);

    return {
        .position = global_transform.position + logical_rot.rotate(mesh_component.position_offset * global_transform.scale),
        .rotation = (logical_rot * r::Quaternion::from_euler(mesh_component.rotation_offset)).to_euler(),
        .scale = global_transform.scale * mesh_component.scale_offset
    };
}

static inline ::Shader mesh_plugin_apply_shader(
    ::Model *model,
    const r::ecs::Optional<r::Material3d> &material_opt,
    r::ecs::Res<r::Shaders> &shaders
) noexcept
{
    ::Shader original_shader = {};

    if (material_opt.ptr && model && model->materialCount > 0) {
        const ::Shader *custom_shader = shaders.ptr->get(material_opt.ptr->get_shader());

        if (custom_shader) {
            original_shader = model->materials[0].shader;
            model->materials[0].shader = *custom_shader;

            mesh_plugin_send_uniforms(*custom_shader, *material_opt.ptr);
        }
    }

    return original_shader;
}

static inline void mesh_plugin_restore_shader(::Model *model, const ::Shader &original_shader) noexcept
{
    if (model && model->materialCount > 0 && original_shader.id != 0) {
        model->materials[0].shader = original_shader;
    }
}

/**
 * System
 */

static void process_mesh_creation_system(
    r::ecs::ResMut<r::Meshes> meshes
) noexcept
{
    /* Process any meshes that were created on worker threads. */
    meshes.ptr->process_pending_meshes();
}

static void mesh_render_system(
    MeshRenderQuery query,
    r::ecs::ResMut<r::Meshes> meshes,
    r::ecs::Res<r::Shaders> shaders
) noexcept
{
    for (const auto &[mesh_comp, transform, material_opt] : query) {
        const r::Transform3d final_transform = mesh_plugin_get_transform3d(*transform.ptr, *mesh_comp.ptr);
        auto *model = meshes.ptr->get(mesh_comp.ptr->id);

        if (!model) {
            continue;
        }

        const auto &original_shader = mesh_plugin_apply_shader(model, material_opt, shaders);

        meshes.ptr->draw(mesh_comp.ptr->id, final_transform.position, final_transform.rotation, final_transform.scale,
            mesh_comp.ptr->color);

        mesh_plugin_restore_shader(model, original_shader);
    }
}

}// namespace

/**
* public MeshPlugin
*/

r::MeshPlugin::MeshPlugin()
{
    /* __ctor__ */
}

r::MeshPlugin::~MeshPlugin()
{
    /* __dtor__ */
}

void r::MeshPlugin::build(r::Application &app)
{
    app.insert_resource(Meshes{})
        .insert_resource(Shaders{})
        .add_systems<process_mesh_creation_system>(Schedule::BEFORE_RENDER_3D)
        .add_systems<mesh_render_system>(Schedule::RENDER_3D);

    Logger::debug("MeshPlugin built");
}
