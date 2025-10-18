#include <R-Engine/Plugins/MeshPlugin.hpp>

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>

/**
 * static helpers
 */

static void mesh_render_system(r::ecs::Query<r::ecs::Ref<r::Mesh3d>, r::ecs::Ref<r::GlobalTransform3d>> query,
    r::ecs::Res<r::Meshes> meshes) noexcept
{
    for (const auto &[mesh_comp, transform] : query) {
        const auto *t3d = transform.ptr;
        meshes.ptr->draw(mesh_comp.ptr->id, t3d->position, t3d->rotation, t3d->scale, mesh_comp.ptr->color);
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
