#pragma once

#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <unordered_map>

namespace r {

/**
 * @brief Alias for mesh handle type to avoid having u32 everywhere
 */
using MeshHandle = u32;
static constexpr inline const MeshHandle MeshInvalidHandle = static_cast<MeshHandle>(-1);

/**
 * @brief Texture Manager for loading & unloading textures
 * @details cache textures to avoid reloading the same texture multiple times
 */
struct R_ENGINE_API TextureManager final {
    public:
        ~TextureManager();

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
        ~MeshEntry();

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
        ~Meshes();

        /**
        * @brief add a new mesh to the manager with a facultative texture path
        */
        MeshHandle add(const ::Mesh &mesh, const std::string &texture_path = "");

        /**
         * @brief add a new model to the manager with a facultative texture path
         */
        MeshHandle add(const ::Model &model, const std::string &texture_path = "");

        /**
        * @brief get a mesh entry by its handle
        */
        const ::Model *get(const u32 handle) const noexcept;

        /**
        * @brief draw a mesh at a given position, scale, and tint color
        */
        void draw(const MeshHandle handle, const Vec3f &position, const f32 scale, const Color tint) const;

        /**
        * @brief remove a mesh from the manager and free its resources
        */
        void remove(const MeshHandle handle);

        /**
        * @brief get a raw const pointer to the internal mesh data vector
        */
        const std::vector<MeshEntry> *data() const;

    private:
        /**
        * @brief internal method to allocate a new mesh entry
        */
        MeshHandle _allocate();
        void _add_texture(MeshEntry &entry, const std::string &texture_path);

        std::vector<MeshEntry> _data;
        TextureManager _texture_manager;
};

struct R_ENGINE_API Mesh3d final {
    public:
        Mesh3d(const MeshHandle mesh_handle, const r::Color &mesh_color = {}) noexcept;

        /**
        * @brief generate a cube mesh centered at the given position with the given size
        */
        static ::Mesh Cube(const f32 size, const Vec3f &center = {0.f, 0.f, 0.f}) noexcept;

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
