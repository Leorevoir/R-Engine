#pragma once

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <R-Engine/Maths/Vec.hpp>

#include <unordered_map>

namespace r {

using MeshHandle = u32;

struct R_ENGINE_API TextureManager final {
    public:
        ~TextureManager();

        const ::Texture2D *load(const std::string &path);
        void unload(const std::string &path);

    private:
        std::unordered_map<std::string, ::Texture2D> _textures;

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

        MeshHandle add(const ::Mesh &mesh, const std::string &texture_path = "");

        const ::Model *get(const u32 handle) const noexcept;

        void draw(const MeshHandle handle, const Vec3f &position, const f32 scale, const Color tint) const;
        void remove(const MeshHandle handle);

        const std::vector<MeshEntry> *data() const;

    private:
        MeshHandle _allocate();

        std::vector<MeshEntry> _data;
        TextureManager _texture_manager;
};

struct R_ENGINE_API Transform3d {
    public:
        r::Vec3f translation = {0.f, 0.f, 0.f};
        r::Vec3f rotation = {0.f, 0.f, 0.f};
        r::Vec3f scale = {1.f, 1.f, 1.f};
};

struct R_ENGINE_API Mesh3d final {
    public:
        Mesh3d(const MeshHandle mesh_handle, const r::Color &mesh_color = {});
        static ::Mesh Cube(const f32 size, const Vec3f &center = {0.f, 0.f, 0.f});
        static ::Mesh Circle(const f32 radius, const u32 slices, const Vec3f &center = {0.f, 0.f, 0.f});

        MeshHandle id = static_cast<MeshHandle>(-1);
        r::Color color;
};

class R_ENGINE_API MeshPlugin final : public Plugin
{
    public:
        MeshPlugin();
        ~MeshPlugin() override;

        void build(Application &app) override;
};

}// namespace r
