#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
* public
*/

r::MeshEntry::MeshEntry(MeshEntry &&other) noexcept
    : model(other.model), texture(other.texture), texture_path(std::move(other.texture_path)), owns_texture(other.owns_texture),
      valid(other.valid)
{
    other.valid = false;
    other.model = {};
}

r::MeshEntry &r::MeshEntry::operator=(MeshEntry &&other) noexcept
{
    if (this != &other) {
        if (valid) {
            UnloadModel(model);
        }

        /* Steal resources from the other object */
        model = other.model;
        texture = other.texture;
        texture_path = std::move(other.texture_path);
        owns_texture = other.owns_texture;
        valid = other.valid;

        /* Neuter the moved-from object */
        other.valid = false;
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
    valid = false;
}
