#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <utility>

/**
* public
*/

r::MeshEntry::MeshEntry(MeshEntry &&other) noexcept
    : cpu_mesh(other.cpu_mesh), model(other.model), texture(other.texture), texture_path(std::move(other.texture_path)),
      owns_texture(other.owns_texture), valid(other.valid)
{
    other.valid = false;
    other.cpu_mesh = {};
    other.model = {};
}

r::MeshEntry &r::MeshEntry::operator=(MeshEntry &&other) noexcept
{
    if (this != &other) {
        if (valid) {
            UnloadModel(model);
            if (cpu_mesh.vertexCount > 0) {
                UnloadMesh(cpu_mesh);
            }
        }

        /* Steal resources from the other object */
        cpu_mesh = other.cpu_mesh;
        model = other.model;
        texture = other.texture;
        texture_path = std::move(other.texture_path);
        owns_texture = other.owns_texture;
        valid = other.valid;

        /* Neuter the moved-from object */
        other.valid = false;
        other.cpu_mesh = {};
        other.model = {};
    }
    return *this;
}

r::MeshEntry::~MeshEntry()
{
    if (!valid) {
        return;
    }
    UnloadModel(model);
    if (cpu_mesh.vertexCount > 0) {
        UnloadMesh(cpu_mesh);
    }
    valid = false;
}
