#pragma once

#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace r {

/**
 * @brief Alias for mesh handle type to avoid having u32 everywhere
 */
using MeshHandle = u32;
static constexpr inline const MeshHandle MeshInvalidHandle = static_cast<MeshHandle>(-1);

/**
 * @brief Identifier for a model to be loaded from a file path.
 * @details Used in the deferred command to distinguish from already-loaded data.
 */
struct ModelIdentifier {
        std::string path;
};

/**
 * @brief Command for deferred mesh creation.
 */
struct R_ENGINE_API MeshCreationCommand {
        std::variant<::Mesh, ::Model, ModelIdentifier> data;
        std::string texture_path;
        MeshHandle handle;
};

/**
 * @brief Texture Manager for loading & unloading textures
 * @details cache textures to avoid reloading the same texture multiple times
 */
struct R_ENGINE_API TextureManager final {
    public:
        TextureManager() = default;
        ~TextureManager();

        TextureManager(const TextureManager &) = delete;
        TextureManager &operator=(const TextureManager &) = delete;
        TextureManager(TextureManager &&) noexcept = default;
        TextureManager &operator=(TextureManager &&) noexcept = default;

        /**
        * @brief load a texture from a file path or get the cached texture if it was already loaded
        */
        const ::Texture2D *load(const std::string &path);

        /**
        * @brief unload a texture from the cache and free its memory
        */
        void unload(const std::string &path);

    private:
        std::unordered_map<std::string, ::Texture2D> _textures;

        /**
         * @brief internal texture loading implementation
         */
        const ::Texture2D *_load(const std::string &path);
};

/**
* @brief Mesh entry structure
* @details holds a mesh, model, texture, and ownership/validity flags.
*/
struct R_ENGINE_API MeshEntry {
    public:
        MeshEntry() = default;
        ~MeshEntry();

        MeshEntry(const MeshEntry &) = delete;
        MeshEntry &operator=(const MeshEntry &) = delete;
        MeshEntry(MeshEntry &&other) noexcept;
        MeshEntry &operator=(MeshEntry &&other) noexcept;

        ::Mesh cpu_mesh{};
        ::Model model{};
        const ::Texture2D *texture{};
        std::string texture_path{};
        bool owns_texture = false;
        bool valid = false;
};

/**
* @brief Meshes manager
* @details manages a collection of MeshEntry objects, allowing addition, retrieval, drawing, and removal of meshes.
*/
struct R_ENGINE_API Meshes final {
    public:
        Meshes() = default;
        ~Meshes();

        Meshes(const Meshes &) = delete;
        Meshes &operator=(const Meshes &) = delete;

        Meshes(Meshes &&other) noexcept;
        Meshes &operator=(Meshes &&other) noexcept;

        /**
        * @brief add a new mesh to the manager with a facultative texture path
        * @details this operation is deferred and will be processed on the main thread.
        */
        MeshHandle add(::Mesh &&mesh, const std::string &texture_path = "");

        /**
         * @brief add a new model to the manager with a facultative texture path
         * @details this operation is deferred and will be processed on the main thread.
         */
        MeshHandle add(::Model &&model, const std::string &texture_path = "");

        /**
         * @brief Queues a model to be loaded from a file path.
         * @details This is the thread-safe way to load models. The actual file loading
         * and GPU upload will happen on the main thread.
         * @param model_path The path to the model file (e.g., .glb, .obj).
         * @param texture_path Optional path to an external texture to apply.
         * @return A handle to the mesh resource.
         */
        MeshHandle add(const std::string &model_path, const std::string &texture_path = "");

        /**
        * @brief get a mesh entry by its handle
        */
        const ::Model *get(const u32 handle) const noexcept;
        ::Model *get(const u32 handle) noexcept;

        /**
        * @brief draw a mesh at a given position, scale, and tint color
        */
        void draw(const MeshHandle handle, const Vec3f &position, const Vec3f &rotation, const Vec3f &scale, const Color tint) const;

        /**
        * @brief remove a mesh from the manager and free its resources
        */
        void remove(const MeshHandle handle);

        /**
        * @brief get a raw const pointer to the internal mesh data vector
        */
        const std::vector<MeshEntry> *data() const;

        /**
         * @brief Processes all pending mesh creation commands.
         * @details This must be called from the main thread.
         */
        void process_pending_meshes();

    private:
        /**
        * @brief internal method to allocate a new mesh entry
        */
        MeshHandle _allocate();
        void _add_texture(MeshEntry &entry, const std::string &texture_path);

        std::vector<MeshEntry> _data;
        std::vector<MeshHandle> _free_handles;
        TextureManager _texture_manager;
        std::vector<MeshCreationCommand> _pending_meshes;
        std::mutex _pending_meshes_mutex;
};

struct R_ENGINE_API Mesh3d final {
    public:
        /**
        * @brief generate a cube mesh centered at the given position with the given size
        */
        static ::Mesh Cube(const f32 size, const Vec3f &center = {0.f, 0.f, 0.f}) noexcept;

        /**
         * @brief generate a sphere mesh with the given radius, number of rings and slices
         */
        static ::Mesh Sphere(const f32 radius, const i32 rings, const i32 slices) noexcept;

        /**
        * @brief generate a sphere mesh centered at the given position with the given radius and number of slices
        */
        static ::Mesh Circle(const f32 radius, const u32 slices, const Vec3f &center = {0.f, 0.f, 0.f}) noexcept;

        /**
        * @brief load a model from a glb file
        */
        static ::Model Glb(const std::string &path) noexcept;

        MeshHandle id = static_cast<MeshHandle>(-1);
        r::Color color;

        /**
         * @brief Visual offsets applied only during rendering.
         * @details These do NOT affect the logical Transform3d of the entity.
         * Useful for fixing models that are imported with incorrect orientation or origin.
         */
        r::Vec3f position_offset = {0.f, 0.f, 0.f};
        r::Vec3f rotation_offset = {0.f, 0.f, 0.f}; /* Euler angles in radians */
        r::Vec3f scale_offset = {1.f, 1.f, 1.f};
};

/**
* @brief Mesh Plugin for R-Engine
* @details provides mesh components and systems
*/
class R_ENGINE_API MeshPlugin final : public Plugin
{
    public:
        MeshPlugin();
        ~MeshPlugin() override;

        void build(Application &app) override;
};

}// namespace r
