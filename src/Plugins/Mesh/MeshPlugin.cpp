#include <R-Engine/Application.hpp>
#include <R-Engine/Maths/Quaternion.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
 * static helpers
 */

static void mesh_render_system(r::ecs::Query<r::ecs::Ref<r::Mesh3d>, r::ecs::Ref<r::GlobalTransform3d>> query,
    r::ecs::Res<r::Meshes> meshes) noexcept
{
    for (const auto &[mesh_comp, transform] : query) {
        const auto *t3d = transform.ptr;
        const auto *mesh = mesh_comp.ptr;

        /* 1. Calculate visual scale (Logical Scale * Visual Offset Scale) */
        r::Vec3f final_scale = t3d->scale * mesh->scale_offset;

        /* 2. Calculate visual rotation (Logical Rotation * Visual Offset Rotation) */
        r::Quaternion logical_rot = r::Quaternion::from_euler(t3d->rotation);
        r::Quaternion offset_rot = r::Quaternion::from_euler(mesh->rotation_offset);
        r::Quaternion final_rot_q = logical_rot * offset_rot;
        r::Vec3f final_rotation = final_rot_q.to_euler();

        /* 3. Calculate visual position */
        /* The position offset must be rotated by the entity's logical rotation and scaled by its logical scale */
        r::Vec3f scaled_offset = mesh->position_offset * t3d->scale;
        r::Vec3f rotated_offset = logical_rot.rotate(scaled_offset);
        r::Vec3f final_position = t3d->position + rotated_offset;

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
}
