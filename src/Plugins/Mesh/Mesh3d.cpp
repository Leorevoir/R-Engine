#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
* public
*/

r::Mesh3d::Mesh3d(const u32 mesh_handle, const r::Color &mesh_color) noexcept : id(mesh_handle), color(mesh_color)
{
    /* __ctor__ */
}

::Mesh r::Mesh3d::Cube(const f32 size, const Vec3f &center) noexcept
{
    const ::Mesh mesh = GenMeshCube(size, size, size);

    for (i32 i = 0; i < mesh.vertexCount; ++i) {
        mesh.vertices[i * 3 + 0] += center.x;
        mesh.vertices[i * 3 + 1] += center.y;
        mesh.vertices[i * 3 + 2] += center.z;
    }
    return mesh;
}

::Mesh r::Mesh3d::Circle(const f32 radius, const u32 slices, const Vec3f &center) noexcept
{
    ::Mesh mesh{};
    const u32 vertex_count = slices + 2;

    mesh.triangleCount = static_cast<i32>(slices);
    mesh.vertexCount = static_cast<i32>(vertex_count);

    mesh.vertices = (f32 *) MemAlloc(vertex_count * 3 * sizeof(f32));
    mesh.normals = (f32 *) MemAlloc(vertex_count * 3 * sizeof(f32));
    mesh.texcoords = (f32 *) MemAlloc(vertex_count * 2 * sizeof(f32));

    mesh.vertices[0] = center.x;
    mesh.vertices[1] = center.y;
    mesh.vertices[2] = center.z;

    mesh.normals[0] = 0.f;
    mesh.normals[1] = 1.f;
    mesh.normals[2] = 0.f;

    mesh.texcoords[0] = 0.5f;
    mesh.texcoords[1] = 0.5f;

    for (u32 i = 0; i <= slices; i++) {
        const f32 angle = (2 * PI * static_cast<f32>(i)) / static_cast<f32>(slices);
        const f32 x = cosf(angle) * radius;
        const f32 z = sinf(angle) * radius;
        const u32 v = (i + 1) * 3;

        mesh.vertices[v + 0] = center.x + x;
        mesh.vertices[v + 1] = center.y;
        mesh.vertices[v + 2] = center.z + z;

        mesh.normals[v + 0] = 0.f;
        mesh.normals[v + 1] = 1.f;
        mesh.normals[v + 2] = 0.f;

        const u32 t = (i + 1) * 2;

        mesh.texcoords[t + 0] = (x / radius + 1.f) * 0.5f;
        mesh.texcoords[t + 1] = (z / radius + 1.f) * 0.5f;
    }

    UploadMesh(&mesh, false);
    return mesh;
}

::Model r::Mesh3d::Glb(const std::string &path) noexcept
{
    if (!path::exists(path)) {
        Logger::error("Mesh3d::Glb: file does not exist: " + path);
        return {};
    }

    return LoadModel(path.c_str());
}
