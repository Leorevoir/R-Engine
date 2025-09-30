#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
 * static helpers
 */

static void mesh_plugin_startup_system()
{
    /* __nothing__ */
}

static void mesh_render_system(r::ecs::Query<r::ecs::Ref<r::Mesh3d>, r::ecs::Ref<r::Transform3d>> query, r::ecs::Res<r::Meshes> meshes)
{
    for (const auto &[mesh_comp, transform] : query) {
        const u32 id = mesh_comp.ptr->id;
        const ::Model *model = meshes.ptr->get(id);

        if (!model) {
            continue;
        }

        const auto t3d = transform.ptr;
        const auto tr = t3d->translation;
        const ::Vector3 pos = {tr.x, tr.y, tr.z};
        const f32 scale = t3d->scale.x;

        DrawModel(*model, pos, scale, RED);
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
    app.insert_resource(Meshes{})
        .add_systems(Schedule::STARTUP, mesh_plugin_startup_system)
        .add_systems(Schedule::RENDER, mesh_render_system);
}
