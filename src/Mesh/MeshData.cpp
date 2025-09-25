#include <R-Engine/Mesh/MeshData.hpp>

/**
* public
*/
// clang-format off

u64 r::mesh::MeshData::vertex_count() const noexcept
{
    return vertices.size();
}

u64 r::mesh::MeshData::triangle_count() const noexcept
{
    switch (topology) {
        case Topology::Triangles:
            return indices.size() / 3;
        case Topology::Lines:
            return indices.size() / 2;
        default:
            return 0;
    }
}

r::mesh::MeshData r::mesh::MeshData::Cube() noexcept
{
    MeshData mesh;

    mesh.reserve_vertices(8);
    mesh.reserve_indices(36);

    mesh.vertices = {
        Vertex{Vec3f{-0.5f, -0.5f, -0.5f}}, ///<< 0
        Vertex{Vec3f{ 0.5f, -0.5f, -0.5f}}, ///<< 1
        Vertex{Vec3f{ 0.5f,  0.5f, -0.5f}}, ///<< 2
        Vertex{Vec3f{-0.5f,  0.5f, -0.5f}}, ///<< 3
        Vertex{Vec3f{-0.5f, -0.5f,  0.5f}}, ///<< 4
        Vertex{Vec3f{ 0.5f, -0.5f,  0.5f}}, ///<< 5
        Vertex{Vec3f{ 0.5f,  0.5f,  0.5f}}, ///<< 6
        Vertex{Vec3f{-0.5f,  0.5f,  0.5f}}  ///<< 7
    };

    mesh.indices = {
        0,1,2, 0,2,3, 4,6,5, 4,7,6,
        4,3,7, 4,0,3, 1,5,6, 1,6,2,
        4,1,0, 4,5,1, 3,2,6, 3,6,7
    };

    mesh.topology = MeshData::Topology::Triangles;

    for (u8 i = 0; i < 8; ++i) {
        mesh.vertices[i].uv = Vec2f{static_cast<f32>(i & 1), static_cast<f32>((i >> 1) & 1)};
    }

    mesh.compute_normals();
    mesh.compute_tangents();

    return mesh;
}

bool r::mesh::MeshData::valid() const noexcept
{
    if (vertices.empty()) {
        return false;
    }
    const auto ic = indices.size();

    switch (topology) {
        case Topology::Triangles:
            if (ic % 3u != 0u) {
                return false;
            }
            break;
        case Topology::Lines:
            if (ic % 2u != 0u) {
                return false;
            }
            break;
        default:
            return false;
    }

    const u64 vc = vertex_count();

    for (u64 i = 0; i < ic; ++i) {
        if (indices[i] >= vc) {
            return false;
        }
    }
    return true;
}

void r::mesh::MeshData::compute_normals() noexcept
{
    if (vertices.empty() || indices.empty() || topology != Topology::Triangles) {
        return;
    }

    const u64 vc = vertex_count();
    const u64 tc = triangle_count();
    const u64 ic = indices.size();

    for (auto &v : vertices) {
        v.normal = Vec3f{0.f, 0.f, 0.f};
    }

    /**
    * @brief Newell's method for computing face normals
    */
    for (u64 t = 0; t < tc; ++t) {
        const u64 base_idx = 3u * t;

        if (base_idx + 2 >= ic) {
            break;
        }

        const u32 i0 = indices[base_idx + 0];
        const u32 i1 = indices[base_idx + 1];
        const u32 i2 = indices[base_idx + 2];

        if (i0 >= vc || i1 >= vc || i2 >= vc) {
            continue;
        }

        const Vec3f &A = vertices[i0].position;
        const Vec3f &B = vertices[i1].position;
        const Vec3f &C = vertices[i2].position;

        const Vec3f AB = B - A;
        const Vec3f AC = C - A;
        const Vec3f N = AB.cross(AC);

        vertices[i0].normal = vertices[i0].normal + N;
        vertices[i1].normal = vertices[i1].normal + N;
        vertices[i2].normal = vertices[i2].normal + N;
    }

    for (auto &v : vertices) {
        const f32 len = v.normal.length();

        if (F32::equal(len, 0.f)) {
            v.normal = Vec3f{0.f, 1.f, 0.f};
        } else {
            v.normal = v.normal / len;
        }
    }

}

void r::mesh::MeshData::compute_tangents() noexcept
{
    if (vertices.empty() || indices.empty() || topology != Topology::Triangles) {
        return;
    }

    compute_normals();

    const u64 vc = vertex_count();
    const u64 tc = triangle_count();

    for (auto &v : vertices) {
        v.tangent = Vec3f{0.f, 0.f, 0.f};
        v.bitangent = Vec3f{0.f, 0.f, 0.f};
    }

    for (u64 t = 0; t < tc; ++t) {
        const u64 base_idx = 3u * t;
        const u32 i0 = indices[base_idx + 0];
        const u32 i1 = indices[base_idx + 1];
        const u32 i2 = indices[base_idx + 2];

        if (i0 >= vc || i1 >= vc || i2 >= vc) {
            continue;
        }

        const Vec3f &v0 = vertices[i0].position;
        const Vec3f &v1 = vertices[i1].position;
        const Vec3f &v2 = vertices[i2].position;

        const Vec2f &uv0 = vertices[i0].uv;
        const Vec2f &uv1 = vertices[i1].uv;
        const Vec2f &uv2 = vertices[i2].uv;

        const Vec3f E1 = v1 - v0;
        const Vec3f E2 = v2 - v0;
        const Vec2f Duv1 = uv1 - uv0;
        const Vec2f Duv2 = uv2 - uv0;

        const f32 denom = (Duv1.x * Duv2.y - Duv2.x * Duv1.y);

        if (F32::equal(denom, 0.f)) {
            continue;
        }

        const f32 f = 1.0f / denom;

        Vec3f tangent;
        Vec3f bitangent;

        tangent.x = f * (Duv2.y * E1.x - Duv1.y * E2.x);
        tangent.y = f * (Duv2.y * E1.y - Duv1.y * E2.y);
        tangent.z = f * (Duv2.y * E1.z - Duv1.y * E2.z);

        bitangent.x = f * (-Duv2.x * E1.x + Duv1.x * E2.x);
        bitangent.y = f * (-Duv2.x * E1.y + Duv1.x * E2.y);
        bitangent.z = f * (-Duv2.x * E1.z + Duv1.x * E2.z);

        vertices[i0].tangent = vertices[i0].tangent + tangent;
        vertices[i1].tangent = vertices[i1].tangent + tangent;
        vertices[i2].tangent = vertices[i2].tangent + tangent;

        vertices[i0].bitangent = vertices[i0].bitangent + bitangent;
        vertices[i1].bitangent = vertices[i1].bitangent + bitangent;
        vertices[i2].bitangent = vertices[i2].bitangent + bitangent;
    }

    /**
    * @brief Gram-Schmidt orthogonalization & normalization
    */
    for (u64 i = 0; i < vc; ++i) {
        Vec3f &t = vertices[static_cast<size_t>(i)].tangent;
        const Vec3f &n = vertices[static_cast<size_t>(i)].normal;

        const f32 ndot = n.dot(t);
        const Vec3f ortho = t - n * ndot;
        const f32 len = ortho.length();

        if (F32::equal(len, 0.f) == false) {
            t = ortho / len;
        } else {
            const Vec3f up = (std::fabs(n.y) < 0.999f) ? Vec3f{0.f, 1.f, 0.f} : Vec3f{1.f, 0.f, 0.f};

            t = n.cross(up);
            const f32 tlen = t.length();

            if (F32::equal(tlen, 0.f) == false) {
                t = t / tlen;
            } else {
                t = Vec3f{1.f, 0.f, 0.f};
            }
        }
}
}

void r::mesh::MeshData::fill_flat_buffers(std::vector<f32> &out_positions, std::vector<f32> &out_normals, std::vector<f32> &out_uvs) const
{
    const u64 vc = vertex_count();

    out_positions.resize(vc * 3);
    out_normals.resize(vc * 3);
    out_uvs.resize(vc * 2);

    f32 *pos_ptr = out_positions.data();
    f32 *norm_ptr = out_normals.data();
    f32 *uv_ptr = out_uvs.data();

    for (u64 i = 0; i < vc; ++i) {
        const auto &v = vertices[i];

        *pos_ptr++ = v.position.x;
        *pos_ptr++ = v.position.y;
        *pos_ptr++ = v.position.z;

        *norm_ptr++ = v.normal.x;
        *norm_ptr++ = v.normal.y;
        *norm_ptr++ = v.normal.z;

        *uv_ptr++ = v.uv.x;
        *uv_ptr++ = v.uv.y;
    }
}

void r::mesh::MeshData::reserve_vertices(const u64 count) noexcept
{
    vertices.reserve(count);
}

void r::mesh::MeshData::reserve_indices(const u64 count) noexcept
{
    indices.reserve(count);
}
