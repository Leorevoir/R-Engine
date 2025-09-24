#include <R-Engine/Mesh/MeshData.hpp>

/**
* public
*/
// clang-format off

u64 r::mesh::MeshData::vertex_count() const noexcept
{
    return positions.size();
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

    mesh.positions = {
        Vec3f{-0.5f, -0.5f, -0.5f}, Vec3f{ 0.5f, -0.5f, -0.5f},
        Vec3f{ 0.5f,  0.5f, -0.5f}, Vec3f{-0.5f,  0.5f, -0.5f},
        Vec3f{-0.5f, -0.5f,  0.5f}, Vec3f{ 0.5f, -0.5f,  0.5f},
        Vec3f{ 0.5f,  0.5f,  0.5f}, Vec3f{-0.5f,  0.5f,  0.5f}
    };
    mesh.indices = {
        0,1,2, 0,2,3, 4,6,5, 4,7,6,
        4,3,7, 4,0,3, 1,5,6, 1,6,2,
        4,1,0, 4,5,1, 3,2,6, 3,6,7
    };
    mesh.topology = Topology::Triangles;
    mesh.compute_normals();

    mesh.uvs.resize(8);
    for (u8 i = 0; i < 8; ++i) {
        mesh.uvs[i] = Vec2f{static_cast<f32>(i & 1), static_cast<f32>((i >> 1) & 1)};
    }
    return mesh;
}

bool r::mesh::MeshData::valid() const noexcept
{
    if (positions.empty()) {
        return false;
    }

    const auto indice_size = indices.size();
    const auto position_size = positions.size();

    switch (topology) {
        case Topology::Triangles:
            if (indice_size % 3 != 0) {
                return false;
            }
            break;
        case Topology::Lines:
            if (indice_size % 2 != 0) {
                return false;
            }
            break;
        default:
            return false;
    }
    if (!normals.empty() && normals.size() != position_size) {
        return false;
    }
    if (!uvs.empty() && uvs.size() != position_size) {
        return false;
    }
    if (!tangents.empty() && tangents.size() != position_size) {
        return false;
    }
    for (u32 i = 0; i < indice_size; ++i) {
        if (indices[i] >= position_size) {
            return false;
        }
    }
    return true;
}

void r::mesh::MeshData::compute_normals() noexcept
{
    if (positions.empty() || indices.empty() || topology != Topology::Triangles) {
        return;
    }

    const u64 vc = vertex_count();
    const u64 tc = triangle_count();
    const u64 ic = indices.size();

    normals.assign(vc, Vec3f{0.f, 0.f, 0.f});

    for (u64 t = 0; t < tc; ++t) {
        const u64 base_idx = 3 * t;

        if (base_idx + 2 >= ic) {
            break;
        }

        const u32 i0 = indices[base_idx];
        const u32 i1 = indices[base_idx + 1];
        const u32 i2 = indices[base_idx + 2];

        if (i0 >= vc || i1 >= vc || i2 >= vc) {
            continue;
        }

        const Vec3f &A = positions[i0];
        const Vec3f &B = positions[i1];
        const Vec3f &C = positions[i2];

        const Vec3f AB = B - A;
        const Vec3f AC = C - A;
        const Vec3f N = AB.cross(AC);

        normals[i0] = normals[i0] + N;
        normals[i1] = normals[i1] + N;
        normals[i2] = normals[i2] + N;
    }

    /** @info normalize & handle zero-length normals */
    for (auto &n : normals) {
        const f32 len = n.length();

        if (F32::equal(len, 0.f)) {
            n = Vec3f{0.f, 1.f, 0.f};
        } else {
            n = n / len;
        }
    }
}

void r::mesh::MeshData::compute_tangents() noexcept
{
    if (positions.empty() || uvs.empty() || indices.empty() || topology != Topology::Triangles) {
        return;
    }

    if (normals.size() != positions.size()) {
        compute_normals();
    }

    const u64 vc = vertex_count();
    const u64 tc = triangle_count();

    tangents.assign(static_cast<size_t>(vc), Vec3f{0.f, 0.f, 0.f});
    bitangents.assign(static_cast<size_t>(vc), Vec3f{0.f, 0.f, 0.f});

    for (u64 t = 0; t < tc; ++t) {
        const u64 base_idx = 3ull * t;
        const u32 i0 = indices[base_idx + 0];
        const u32 i1 = indices[base_idx + 1];
        const u32 i2 = indices[base_idx + 2];

        if (i0 >= vc || i1 >= vc || i2 >= vc) {
            continue;
        }

        const Vec3f &v0 = positions[i0];
        const Vec3f &v1 = positions[i1];
        const Vec3f &v2 = positions[i2];

        const Vec2f &uv0 = uvs[i0];
        const Vec2f &uv1 = uvs[i1];
        const Vec2f &uv2 = uvs[i2];

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

        tangents[static_cast<size_t>(i0)] += tangent;
        tangents[static_cast<size_t>(i1)] += tangent;
        tangents[static_cast<size_t>(i2)] += tangent;

        bitangents[static_cast<size_t>(i0)] += bitangent;
        bitangents[static_cast<size_t>(i1)] += bitangent;
        bitangents[static_cast<size_t>(i2)] += bitangent;
    }

    /** @brief Gram-Schmidt orthogonalization & normalization for tangents */
    for (u64 i = 0; i < vc; ++i) {
        Vec3f &t = tangents[static_cast<size_t>(i)];
        const Vec3f &n = normals[static_cast<size_t>(i)];

        const f32 ndot = n.dot(t);
        Vec3f ortho = t - n * ndot;
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
                t = Vec3f{1.f, 0.f, 0.f}; ///<< last resort but should never happen
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

    const bool has_normals = (normals.size() == vc);
    const bool has_uvs = (uvs.size() == vc);

    for (u64 i = 0; i < vc; ++i) {
        const Vec3f &pos = positions[i];

        *pos_ptr++ = pos.x;
        *pos_ptr++ = pos.y;
        *pos_ptr++ = pos.z;

        if (has_normals) {
            const Vec3f &norm = normals[i];

            *norm_ptr++ = norm.x;
            *norm_ptr++ = norm.y;
            *norm_ptr++ = norm.z;

        } else {
            *norm_ptr++ = 0.f;
            *norm_ptr++ = 0.f;
            *norm_ptr++ = 0.f;
        }

        if (has_uvs) {
            const Vec2f &uv = uvs[i];

            *uv_ptr++ = uv.x;
            *uv_ptr++ = uv.y;

        } else {
            *uv_ptr++ = 0.f;
            *uv_ptr++ = 0.f;
        }
    }
}

void r::mesh::MeshData::reserve_vertices(const u64 count) noexcept
{
    positions.reserve(count);
    normals.reserve(count);
    uvs.reserve(count);
}

void r::mesh::MeshData::reserve_indices(const u64 count) noexcept
{
    indices.reserve(count);
}
