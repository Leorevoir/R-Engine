#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Maths/Quaternion.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>
#include <cmath>

/**
 * public Meshes
 */

r::Meshes::Meshes(Meshes &&other) noexcept
{
    std::lock_guard<std::mutex> lock(other._pending_meshes_mutex);
    _pending_meshes = std::move(other._pending_meshes);
    _data = std::move(other._data);
    _free_handles = std::move(other._free_handles);
    _texture_manager = std::move(other._texture_manager);
}

r::Meshes &r::Meshes::operator=(Meshes &&other) noexcept
{
    if (this != &other) {
        std::scoped_lock lock(_pending_meshes_mutex, other._pending_meshes_mutex);
        _pending_meshes = std::move(other._pending_meshes);
        _data = std::move(other._data);
        _free_handles = std::move(other._free_handles);
        _texture_manager = std::move(other._texture_manager);
    }
    return *this;
}

r::Meshes::~Meshes()
{
    _data.clear();
}

r::MeshHandle r::Meshes::add(::Mesh &&mesh, const std::string &texture_path)
{
    if (mesh.vertexCount == 0 || !mesh.vertices) {
        Logger::error("Failed to queue mesh for creation: invalid mesh data");
        UnloadMesh(mesh);
        return MeshInvalidHandle;
    }

    const r::MeshHandle handle = _allocate();
    {
        std::lock_guard<std::mutex> lock(_pending_meshes_mutex);
        _pending_meshes.push_back({std::move(mesh), texture_path, handle});
    }
    return handle;
}

r::MeshHandle r::Meshes::add(::Model &&model, const std::string &texture_path)
{
    if (model.meshCount == 0 || !model.meshes) {
        Logger::error("Failed to queue model for creation: invalid model data");
        UnloadModel(model);
        return MeshInvalidHandle;
    }

    const r::MeshHandle handle = _allocate();
    {
        std::lock_guard<std::mutex> lock(_pending_meshes_mutex);
        _pending_meshes.push_back({std::move(model), texture_path, handle});
    }
    return handle;
}

r::MeshHandle r::Meshes::add(const std::string &model_path, const std::string &texture_path)
{
    if (model_path.empty()) {
        Logger::error("Failed to queue model for creation: model path is empty.");
        return MeshInvalidHandle;
    }

    const r::MeshHandle handle = _allocate();
    {
        std::lock_guard<std::mutex> lock(_pending_meshes_mutex);
        _pending_meshes.push_back({ModelIdentifier{model_path}, texture_path, handle});
    }
    return handle;
}

void r::Meshes::process_pending_meshes()
{
    std::vector<MeshCreationCommand> commands_to_process;
    {
        std::lock_guard<std::mutex> lock(_pending_meshes_mutex);
        if (_pending_meshes.empty()) {
            return;
        }
        commands_to_process.swap(_pending_meshes);
    }

    for (auto &command : commands_to_process) {
        auto &entry = _data[command.handle];

        std::visit(
            [&](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ::Mesh>) {
                    /* The Model will point to the data inside cpu_mesh,
                    so we need to store it here. */
                    entry.cpu_mesh = arg;
                    entry.model = LoadModelFromMesh(entry.cpu_mesh);
                } else if constexpr (std::is_same_v<T, ::Model>) {
                    /* The model is self-contained. */
                    entry.model = std::move(arg);
                } else if constexpr (std::is_same_v<T, ModelIdentifier>) {
                    /* The model is loaded from path here, on the main thread. */
                    entry.model = LoadModel(arg.path.c_str());
                    if (entry.model.meshCount == 0) {
                        Logger::error("Failed to process deferred model from path: " + arg.path);
                        /* The entry will remain invalid by default. */
                        return;
                    }
                }
            },
            std::move(command.data));

        if (entry.model.meshCount > 0) {
            if (!command.texture_path.empty()) {
                _add_texture(entry, command.texture_path);
            }
            entry.valid = true;
            Logger::debug("Processed deferred mesh for handle: " + std::to_string(command.handle));
        } else {
            entry.valid = false;
        }
    }
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

    e = MeshEntry{};
    e.valid = false;
    _free_handles.push_back(handle);
}
/**
* private
*/

r::MeshHandle r::Meshes::_allocate()
{
    if (!_free_handles.empty()) {
        MeshHandle handle = _free_handles.back();
        _free_handles.pop_back();
        _data[handle] = MeshEntry{};
        return handle;
    }

    MeshHandle handle = static_cast<MeshHandle>(_data.size());
    _data.emplace_back();
    return handle;
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
