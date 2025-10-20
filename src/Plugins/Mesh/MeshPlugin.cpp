#include <R-Engine/Plugins/MeshPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Maths/Quaternion.hpp>

/**
 * static helpers
 */

static void mesh_render_system(r::ecs::Query<r::ecs::Ref<r::Mesh3d>, r::ecs::Ref<r::GlobalTransform3d>> query,
    r::ecs::Res<r::Meshes> meshes) noexcept
{
    for (const auto &[mesh_comp, transform] : query) {
        const auto *t3d = transform.ptr;
        const auto *mesh = mesh_comp.ptr;

        const r::Vec3f final_scale = t3d->scale * mesh->scale_offset;

        const r::Quaternion logical_rot = r::Quaternion::from_euler(t3d->rotation);
        const r::Quaternion offset_rot = r::Quaternion::from_euler(mesh->rotation_offset);
        const r::Quaternion final_rot_q = logical_rot * offset_rot;
        const r::Vec3f final_rotation = final_rot_q.to_euler();
        const r::Vec3f scaled_offset = mesh->position_offset * t3d->scale;
        const r::Vec3f rotated_offset = logical_rot.rotate(scaled_offset);
        const r::Vec3f final_position = t3d->position + rotated_offset;

        meshes.ptr->draw(mesh->id, final_position, final_rotation, final_scale, mesh->color);
    }
}

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
    app.insert_resource(Meshes{}).add_systems<mesh_render_system>(Schedule::RENDER_3D);
    Logger::debug("MeshPlugin built");
}
