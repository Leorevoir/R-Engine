#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
 * public MeshEntry
 */

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

/**
 * public Meshes
 */

r::Meshes::~Meshes()
{
    _data.clear();
}

u32 r::Meshes::add(const ::Mesh &mesh, const std::string &texture_path)
{
    u32 handle = _allocate();
    auto &entry = _data[handle];

    entry.cpu_mesh = mesh;
    entry.model = LoadModelFromMesh(entry.cpu_mesh);

    /** @info initialize cpu_mesh to avoid double free on destruction */
    entry.cpu_mesh.vertexCount = 0;
    entry.cpu_mesh.triangleCount = 0;
    entry.cpu_mesh.vertices = nullptr;
    entry.cpu_mesh.normals = nullptr;
    entry.cpu_mesh.texcoords = nullptr;
    entry.cpu_mesh.indices = nullptr;

    /** @info load texture if a path is provided using the Texture Manager */
    if (!texture_path.empty()) {
        _add_texture(entry, (texture_path));
    }

    entry.valid = true;
    Logger::info("bind mesh with handle: " + std::to_string(handle));
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

void r::Meshes::draw(const u32 handle, const Vec3f &position, const f32 scale, const Color tint) const
{
    const ::Model *m = get(handle);

    if (!m) {
        return;
    }
    DrawModel(*m, {position.x, position.y, position.z}, scale, {tint.r, tint.g, tint.b, tint.a});
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

    if (!e.texture_path.empty()) {
        _texture_manager.unload(e.texture_path);
        e.texture_path.clear();
        e.texture = nullptr;
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

void r::Meshes::_add_texture(MeshEntry &entry, const std::string &texture_path)
{
    const auto &texture = _texture_manager.load(texture_path);

    if (!texture) {
        Logger::error("Failed to bind texture to mesh: " + texture_path);
        return;
    }

    entry.texture = texture;
    entry.texture_path = texture_path;
    if (entry.model.materialCount > 0) {
        entry.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *entry.texture;
    }
}
