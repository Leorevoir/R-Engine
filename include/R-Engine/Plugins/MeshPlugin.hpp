#pragma once

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <R-Engine/Maths/Vec.hpp>

namespace r {

struct R_ENGINE_API MeshEntry {
    public:
        ~MeshEntry();

        ::Mesh cpu_mesh{};
        ::Model model{};
        ::Texture2D texture{};
        bool owns_texture = false;
        bool valid = false;
};

struct R_ENGINE_API Meshes final {
    public:
        ~Meshes();

        u32 add(const ::Mesh &mesh, const ::Texture2D *texture = nullptr);
        u32 add(const ::Mesh &mesh, const std::string &texture_path);

        const ::Model *get(const u32 handle) const noexcept;

        void draw(const u32 handle, const Vec3f &position, const f32 scale, const ::Color tint = WHITE) const;
        void remove(const u32 handle);

        const std::vector<MeshEntry> *data() const;

    private:
        u32 _allocate();

        std::vector<MeshEntry> _data;
};

struct R_ENGINE_API Transform3d {
    public:
        r::Vec3f translation = {0.f, 0.f, 0.f};
        r::Vec3f rotation = {0.f, 0.f, 0.f};
        r::Vec3f scale = {1.f, 1.f, 1.f};
};

struct R_ENGINE_API Mesh3d final {
    public:
        Mesh3d(const u32 mesh_handle = 0);
        static ::Mesh Cube(const f32 size, const Vec3f &center = {0.f, 0.f, 0.f});
        static ::Mesh Circle(const f32 radius, const u32 slices, const Vec3f &center = {0.f, 0.f, 0.f});

        u32 id = static_cast<u32>(-1);
};

class R_ENGINE_API MeshPlugin final : public Plugin
{
    public:
        MeshPlugin();
        ~MeshPlugin() override;

        void build(Application &app) override;
};

}// namespace r
