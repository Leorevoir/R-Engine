#pragma once

#include <R-Engine/Mesh/MeshVertex.hpp>

#include <vector>

namespace r::mesh {

/**
* @brief CPU side mesh data representation:
*
*   - vertices
*   - indices
*
* @details
*   - compute_normals() -> fills `normals` from `positions` and `indices`
*   - compute_tangents() -> fills `tangents` from `positions`, `uvs` and `indices`
*   - fill_flat_buffers() -> fills flat arrays from the mesh data for easy upload to GPU
*/
struct MeshData {

    public:
        std::vector<Vertex> vertices;
        std::vector<u32> indices;

        enum class Topology { Triangles, Lines } topology = Topology::Triangles;
        static constexpr float Invalid = std::numeric_limits<float>::quiet_NaN();

        MeshData() = default;
        ~MeshData() = default;

        /**
         * @brief creates a cube mesh with indixed triangles
         * @details
         *
         *  - vertices at ±0.5f in all axis
         *  - 8 unique vertices & 12 triangles -> 36 indices
         *  - computed normals & uvs
         *
         * @vertex
         *
         *         7_____________________6
         *        /|                    /|
         *       / |                   / |
         *      3---------------------2  |
         *      |  |                  |  |
         *      |  |                  |  |
         *      |  |                  |  |
         *      |  |                  |  |
         *      | 5-------------------|-4
         *      |/                    |/        
         *      1---------------------0         
         */
        static MeshData Cube() noexcept;

        /**
        * @brief returns the number of vertices in the mesh (positions.size())
         */
        u64 vertex_count() const noexcept;

        /**
         * @brief returns the number of triangles in the mesh
         */
        u64 triangle_count() const noexcept;

        /**
         * @brief checks if the mesh data is valid
         * @details
         *  - positions array is not empty
        *   - indices count matches the topology (multiple of 3 for triangles, multiple of 2 for lines)
        *   - normals | uvs | tangents are either empty or match the positions count
        *   - all indices are in range of positions count
         */
        bool valid() const noexcept;

        /**
         * @brief compute vertex noramls from triangle geometry
         * @details smooth vertex noramls by averaging face noramls of adjacent triangles
         *
         *          C
         *         /\
         *        /  \    
         *       /    \     AB = B - A
         *      /      \    AC = C - A  
         *     /________\   N = AB × AC (cross product)
         *    A          B
         *
         * vertex normal accumulation:
         *
         *     Triangle 1    Triangle 2    Triangle 3
         *         N1           N2            N3
         *         .            .             .
         *        / \          / \           / \
         *       /   \        /   \         /   \
         *      ○-----○      ○-----○       ○-----○
         *
         * vertex normal = normalize(N1 + N2 + N3)
         */
        void compute_normals() noexcept;

        /**
         * @brief compute tangent space vectors for normal mapping
         * 
         * tangent space construction per triangle:
         * 
         *    3D Space:          texture space:
         *        C                   ↑
         *       /\                   | 
         *      /  \                  |   (uC,vC)
         *     /    \                 |    C
         *    /      \                |   / \
         *   /________\               |  /   \
         *  A          B              | A_____B → u
         *                            | (uA,vA) (uB,vB)
         * 
         * vector relationships:
         * 
         *     E1 = B - A       (3D edge vector 1)
         *     E2 = C - A       (3D edge vector 2)
         *     ΔUV1 = (uB-uA, vB-vA)
         *     ΔUV2 = (uC-uA, vC-vA)
         * 
         * solve: [E1] = [Δu1  Δv1] × [T]
         *        [E2]   [Δu2  Δv2]   [B]
         * 
         * resulting tangent space:
         * 
         *        N (normal)
         *        ↑
         *        |       T (tangent) → texture u-axis
         *        |      ╱
         *        |     ╱ 
         *        |    ╱ B (bitangent) → texture v-axis
         *        |   ╱
         *        |  ╱
         *        | ╱
         *        ○─────→
         */
        void compute_tangents() noexcept;

        /**
        * @brief fills flat buffers from the mesh data for easy upload to GPU
        * @details arrays are always filled missing data is filled with 0.f
        * @param out_positions flat array of positions (3 floats per vertex)
        * @param out_normals flat array of normals (3 floats per vertex)
        * @param out_uvs flat array of uvs (2 floats per vertex)
        */
        void fill_flat_buffers(std::vector<f32> &out_positions, std::vector<f32> &out_normals, std::vector<f32> &out_uvs) const;

        /**
        * @brief memory management encapsulated bc of static ctor
        */
        void reserve_vertices(const u64 count) noexcept;

        /**
        * @brief memory management encapsulated bc of static ctor
        */
        void reserve_indices(const u64 count) noexcept;
};

}// namespace r::mesh
