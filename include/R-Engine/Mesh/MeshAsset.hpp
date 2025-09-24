#pragma once

#include <R-Engine/Mesh/MeshData.hpp>

#include <optional>

namespace r::mesh {

struct GpuMeshHandle {
    public:
        u32 id = 0;
};

/**
* @brief MeshAsset contains both CPU and GPU representations of a mesh
* @details this struct holds the CPU-side mesh data and an optional GPU handle
*
* @param dirty flag indicates whether the CPU data has changed and needs to be re-uploaded to the GPU
*/
struct MeshAsset {
    public:
        MeshData cpu;
        std::optional<GpuMeshHandle> gpu;
        bool dirty = true;
};

}// namespace r::mesh
