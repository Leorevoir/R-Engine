#include <R-Engine/Plugins/MeshPlugin.hpp>

r::MeshEntry::~MeshEntry()
{
    if (!valid) {
        return;
    }
    UnloadModel(model);
    if (cpu_mesh.vertexCount > 0) {
        UnloadMesh(cpu_mesh);
    }
    if (owns_texture) {
        UnloadTexture(texture);
    }
    valid = false;
}

/**
 * public Meshes
 */

r::Meshes::~Meshes()
{
    _data.clear();
}

u32 r::Meshes::add(const ::Mesh &mesh, const ::Texture2D *texture)
{
    u32 handle = _allocate();
    auto &entry = _data[handle];

    entry.cpu_mesh = mesh;
    entry.model = LoadModelFromMesh(entry.cpu_mesh);

    entry.cpu_mesh.vertexCount = 0;
    entry.cpu_mesh.triangleCount = 0;
    entry.cpu_mesh.vertices = nullptr;
    entry.cpu_mesh.normals = nullptr;
    entry.cpu_mesh.texcoords = nullptr;
    entry.cpu_mesh.indices = nullptr;

    entry.valid = true;

    if (texture) {
        entry.texture = *texture;
        entry.owns_texture = true;
        if (entry.model.materialCount > 0) {
            entry.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = entry.texture;
        }
    }
    return handle;
}

u32 r::Meshes::add(const ::Mesh &mesh, const std::string &texture_path)
{
    ::Texture2D tex = LoadTexture(texture_path.c_str());
    u32 handle = add(mesh, &tex);

    return handle;
}

const ::Model *r::Meshes::get(const u32 handle) const noexcept
{
    if (handle >= _data.size()) {
        return nullptr;
    }

    const auto &e = _data[handle];

    if (!e.valid) {
        return nullptr;
    }
    return &e.model;
}

const std::vector<r::MeshEntry> *r::Meshes::data() const
{
    return &_data;
}

void r::Meshes::draw(const u32 handle, const Vec3f &position, const f32 scale, const ::Color tint) const
{
    const ::Model *m = get(handle);

    if (!m) {
        return;
    }
    DrawModel(*m, {position.x, position.y, position.z}, scale, tint);
}

void r::Meshes::remove(const u32 handle)
{
    if (handle >= _data.size()) {
        return;
    }
    auto &e = _data[handle];

    if (!e.valid) {
        return;
    }

    UnloadModel(e.model);
    if (e.cpu_mesh.vertexCount > 0) {
        UnloadMesh(e.cpu_mesh);
    }
    if (e.owns_texture) {
        UnloadTexture(e.texture);
    }
    e.valid = false;
}

/**
* private
*/

u32 r::Meshes::_allocate()
{
    _data.emplace_back();
    return static_cast<u32>(_data.size() - 1);
}
