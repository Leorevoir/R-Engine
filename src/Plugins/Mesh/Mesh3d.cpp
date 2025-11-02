#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Maths/Quaternion.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

#include <cstring>

/**
* public
*/

::Mesh r::Mesh3d::Cube(const f32 size, const Vec3f &center) noexcept
{
    ::Mesh mesh{};
    mesh.vertexCount = 24;
    mesh.triangleCount = 12;

    mesh.vertices = (float *) MemAlloc(static_cast<unsigned int>(static_cast<size_t>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.texcoords = (float *) MemAlloc(static_cast<unsigned int>(static_cast<size_t>(mesh.vertexCount) * 2 * sizeof(float)));
    mesh.normals = (float *) MemAlloc(static_cast<unsigned int>(static_cast<size_t>(mesh.vertexCount) * 3 * sizeof(float)));
    mesh.indices =
        (unsigned short *) MemAlloc(static_cast<unsigned int>(static_cast<size_t>(mesh.triangleCount) * 3 * sizeof(unsigned short)));

    float half_size = size / 2.0f;

    float p_vertices[] = {/* Front face */
        -half_size, -half_size, half_size, half_size, -half_size, half_size, half_size, half_size, half_size, -half_size, half_size,
        half_size,
        /* Back face */
        -half_size, -half_size, -half_size, -half_size, half_size, -half_size, half_size, half_size, -half_size, half_size, -half_size,
        -half_size,
        /* Top face */
        -half_size, half_size, -half_size, -half_size, half_size, half_size, half_size, half_size, half_size, half_size, half_size,
        -half_size,
        /* Bottom face */
        -half_size, -half_size, -half_size, half_size, -half_size, -half_size, half_size, -half_size, half_size, -half_size, -half_size,
        half_size,
        /* Right face */
        half_size, -half_size, -half_size, half_size, half_size, -half_size, half_size, half_size, half_size, half_size, -half_size,
        half_size,
        /* Left face */
        -half_size, -half_size, -half_size, -half_size, -half_size, half_size, -half_size, half_size, half_size, -half_size, half_size,
        -half_size};

    float p_normals[] = {
        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,    ///< Front
        0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,///< Back
        0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0,    ///< Top
        0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0,//< Bottom
        1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,    ///< Right
        -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0 ///< Left
    };

    float p_texcoords[] = {
        0, 0, 1, 0, 1, 1, 0, 1,///< Front
        1, 0, 1, 1, 0, 1, 0, 0,///< Back
        0, 1, 0, 0, 1, 0, 1, 1,///< Top
        1, 1, 0, 1, 0, 0, 1, 0,///< Bottom
        1, 0, 1, 1, 0, 1, 0, 0,///< Right
        0, 0, 1, 0, 1, 1, 0, 1 ///< Left
    };

    unsigned short p_indices[] = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23};

    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.vertices[i * 3 + 0] = p_vertices[i * 3 + 0] + center.x;
        mesh.vertices[i * 3 + 1] = p_vertices[i * 3 + 1] + center.y;
        mesh.vertices[i * 3 + 2] = p_vertices[i * 3 + 2] + center.z;
    }

    memcpy(mesh.normals, p_normals, static_cast<size_t>(mesh.vertexCount) * 3 * sizeof(float));
    memcpy(mesh.texcoords, p_texcoords, static_cast<size_t>(mesh.vertexCount) * 2 * sizeof(float));
    memcpy(mesh.indices, p_indices, static_cast<size_t>(mesh.triangleCount) * 3 * sizeof(unsigned short));

    return mesh;
}

::Mesh r::Mesh3d::Circle(const f32 radius, const u32 slices, const Vec3f &center) noexcept
{
    ::Mesh mesh{};
    const u32 vertex_count = slices + 2;

    mesh.triangleCount = static_cast<i32>(slices);
    mesh.vertexCount = static_cast<i32>(vertex_count);

    mesh.vertices = (f32 *) MemAlloc(static_cast<unsigned int>(vertex_count * 3 * sizeof(f32)));
    mesh.normals = (f32 *) MemAlloc(static_cast<unsigned int>(vertex_count * 3 * sizeof(f32)));
    mesh.texcoords = (f32 *) MemAlloc(static_cast<unsigned int>(vertex_count * 2 * sizeof(f32)));
    mesh.indices = (unsigned short *) MemAlloc(static_cast<unsigned int>(slices * 3 * sizeof(unsigned short)));

    mesh.vertices[0] = center.x;
    mesh.vertices[1] = center.y;
    mesh.vertices[2] = center.z;

    mesh.normals[0] = 0.f;
    mesh.normals[1] = 1.f;
    mesh.normals[2] = 0.f;

    mesh.texcoords[0] = 0.5f;
    mesh.texcoords[1] = 0.5f;

    for (u32 i = 0; i <= slices; i++) {
        const f32 angle = (2 * r::R_PI * static_cast<f32>(i)) / static_cast<f32>(slices);
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

    /* Populate the indices to form triangles. */
    for (u32 i = 0; i < slices; i++) {
        const u32 idx = i * 3;
        mesh.indices[idx + 0] = 0;
        mesh.indices[idx + 1] = static_cast<unsigned short>(i + 1);
        mesh.indices[idx + 2] = static_cast<unsigned short>(i + 2);
    }

    // UploadMesh(&mesh, false);
    return mesh;
}

::Mesh r::Mesh3d::Sphere(const f32 radius, const i32 rings, const i32 slices) noexcept
{
    ::Mesh mesh = GenMeshSphere(radius, rings, slices);

    // UploadMesh(&mesh, false);
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
