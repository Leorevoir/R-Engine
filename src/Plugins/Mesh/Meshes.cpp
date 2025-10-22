#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Maths/Quaternion.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <cmath>

/**
 * public Meshes
 */

r::Meshes::~Meshes()
{
    _data.clear();
}

r::MeshHandle r::Meshes::add(const ::Mesh &mesh, const std::string &texture_path)
{
    if (mesh.vertexCount == 0 || !mesh.vertices) {
        Logger::error("Failed to bind mesh: invalid mesh data");
        return MeshInvalidHandle;
    }

    const r::MeshHandle handle = _allocate();
    auto &entry = _data[handle];

    entry.cpu_mesh = mesh;
    entry.model = LoadModelFromMesh(entry.cpu_mesh);

    /** @debug initialize cpu_mesh to avoid double free on destruction */
    entry.cpu_mesh.vertexCount = 0;
    entry.cpu_mesh.triangleCount = 0;
    entry.cpu_mesh.vertices = nullptr;
    entry.cpu_mesh.normals = nullptr;
    entry.cpu_mesh.texcoords = nullptr;
    entry.cpu_mesh.indices = nullptr;

    /** @debug load texture if a path is provided using the Texture Manager */
    if (!texture_path.empty()) {
        _add_texture(entry, (texture_path));
    }

    entry.valid = true;
    Logger::debug("bind mesh with handle: " + std::to_string(handle));
    return handle;
}

r::MeshHandle r::Meshes::add(const ::Model &model, const std::string &texture_path)
{
    if (model.meshCount == 0) {
        Logger::error("Failed to bind model: invalid model data: mesh count is zero");
        return MeshInvalidHandle;
    }
    if (!model.meshes) {
        Logger::error("Failed to bind model: invalid model data: meshes pointer is null");
        return MeshInvalidHandle;
    }

    const r::MeshHandle handle = _allocate();
    auto &entry = _data[handle];

    entry.model = model;

    /** @debug load texture if a path is provided using the Texture Manager */
    if (!texture_path.empty()) {
        _add_texture(entry, (texture_path));
    }

    entry.valid = true;
    Logger::debug("bind model with handle: " + std::to_string(handle));
    return handle;
}

::Model *r::Meshes::get(const r::MeshHandle handle) noexcept
{
    if (handle >= _data.size()) {
        return nullptr;
    }

    auto &e = _data[handle];

    if (!e.valid) {
        return nullptr;
    }
    return &e.model;
}

const ::Model *r::Meshes::get(const r::MeshHandle handle) const noexcept
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

void r::Meshes::draw(const r::MeshHandle handle, const Vec3f &position, const Vec3f &rotation, const Vec3f &scale, const Color tint) const
{
    const ::Model *model_ptr = get(handle);
    if (!model_ptr) {
        return;
    }

    /* Convert Euler angles to our engine's Quaternion */
    const r::Quaternion q = r::Quaternion::from_euler(rotation);

    /* Convert Quaternion to Axis-Angle for Raylib's DrawModelEx */
    float angle_rad = 2.0f * acosf(q.w);
    float angle_deg = angle_rad * (180.0f / r::R_PI);
    r::Vec3f axis = {1.0f, 0.0f, 0.0f}; /* Default axis if no rotation */

    float s = sqrtf(1.0f - q.w * q.w);
    if (s >= 0.001f) {
        axis.x = q.x / s;
        axis.y = q.y / s;
        axis.z = q.z / s;
    }

    /* Prepare Raylib-compatible structs */
    ::Vector3 rl_position = {position.x, position.y, position.z};
    ::Vector3 rl_rotation_axis = {axis.x, axis.y, axis.z};
    ::Vector3 rl_scale = {scale.x, scale.y, scale.z};
    ::Color rl_tint = {tint.r, tint.g, tint.b, tint.a};

    /* Draw the model using extended parameters, avoiding direct matrix manipulation */
    DrawModelEx(*model_ptr, rl_position, rl_rotation_axis, angle_deg, rl_scale, rl_tint);
}

void r::Meshes::remove(const r::MeshHandle handle)
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

r::MeshHandle r::Meshes::_allocate()
{
    _data.emplace_back();
    return static_cast<r::MeshHandle>(_data.size() - 1);
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
