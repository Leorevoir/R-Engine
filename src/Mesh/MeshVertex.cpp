#include "R-Engine/Mesh/MeshVertex.hpp"

/**
* public
*/

r::mesh::Vertex::Vertex(const Vec3f &in_pos, const Vec3f &in_norm, const Vec2f &in_uv) noexcept
    : position(in_pos), normal(in_norm), uv(in_uv)
{
    /* __ctor__ */
}

r::mesh::Vertex::Vertex(const Vec3f &in_pos, const Vec2f &in_uv) noexcept : position(in_pos), uv(in_uv)
{
    /* __ctor__ */
}

r::mesh::Vertex::Vertex(const Vec3f &in_pos) noexcept : position(in_pos)
{
    /* __ctor__ */
}

/**
* private
*/
