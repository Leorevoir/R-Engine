#pragma once

#include <R-Engine/Mesh/MeshData.hpp>

#include <optional>

namespace r {

namespace mesh {

struct R_ENGINE_API GpuMeshHandle {
    public:
        u32 id = 0;
};

}// namespace mesh

/**
* @brief Mesh contains both CPU and GPU representations of a mesh
* @details this struct holds the CPU-side mesh data and an optional GPU handle
*
* @param dirty flag indicates whether the CPU data has changed and needs to be re-uploaded to the GPU
*/
struct R_ENGINE_API Mesh {
    public:
        mesh::MeshData cpu;
        std::optional<mesh::GpuMeshHandle> gpu;
        bool dirty = true;
};

namespace assets {
template<typename T>
class Handle;
}

using MeshHandle = assets::Handle<Mesh>;

}// namespace r
