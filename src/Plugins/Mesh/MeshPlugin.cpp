#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
 * static helpers
 */

static void mesh_render_system(r::ecs::Query<r::ecs::Ref<r::Mesh3d>, r::ecs::Ref<r::Transform3d>> query,
    r::ecs::Res<r::Meshes> meshes) noexcept
{
    for (const auto &[mesh_comp, transform] : query) {
        const auto *t3d = transform.ptr;
        const auto pos = t3d->translation;
        const auto scale = t3d->scale.x;

        meshes.ptr->draw(mesh_comp.ptr->id, pos, scale, mesh_comp.ptr->color);
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
    app.insert_resource(Meshes{}).add_systems(Schedule::RENDER, mesh_render_system);
}
