#pragma once

#include <R-Engine/Assets/Handle.hpp>
#include <R-Engine/Mesh/MeshAsset.hpp>

namespace r {

using MeshHandle = assets::Handle<mesh::MeshAsset>;

/**
* @brief Material3d is a texture & shader id placeholder
*/
struct Material3d {
    public:
        u32 texture_id = 0;
        u32 shader_id = 0;
};

/**
* @brief Mesh2d is a 2D mesh
*/
struct Mesh2d {
    public:
        MeshHandle mesh;
        explicit Mesh2d(const MeshHandle m = {});
};

/**
* @brief Mesh3d is you guess it a 3D mesh
*/
struct Mesh3d {
    public:
        MeshHandle mesh;
        explicit Mesh3d(const MeshHandle m = {});
};

}// namespace r
