#pragma once

#include <R-Engine/Maths/Vec.hpp>

namespace r::mesh {

/**
 * @brief CPU side vertex representation
 *
 *  - position
 *  - normal
 *  - uv
 *  - tangent
 *  - bitangent
 */
struct R_ENGINE_API Vertex {
    public:
        Vec3f position{};
        Vec3f normal{};
        Vec2f uv{};
        Vec3f tangent{};
        Vec3f bitangent{};

        Vertex() = default;

        /**
         * @brief construct a vertex with position, normal, & uv coordinates
         * @param in_pos position of the vertex
         * @param in_norm normal of the vertex
         * @param in_uv UV coordinates of the vertex
         */
        Vertex(const Vec3f &in_pos, const Vec3f &in_norm, const Vec2f &in_uv) noexcept;

        /**
         * @brief construct a vertex with position and uv coordinates
         * @param in_pos position of the vertex
         * @param in_uv UV coordinates of the vertex
         */
        Vertex(const Vec3f &in_pos, const Vec2f &in_uv) noexcept;

        /**
        * @brief construct a vertex with position only
        * @param in_pos position of the vertex
        */
        Vertex(const Vec3f &in_pos) noexcept;
};

}// namespace r::mesh
