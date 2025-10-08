#include <R-Engine/Components/Transform3d.hpp>

/**
* public
*/

/**
* static helper
*/

static r::Vec3f _apply_rotation(const r::Vec3f &v, const r::Vec3f &rotation) noexcept
{
    /**
    * Euler xyz rotation
    */

    const float cx = std::cos(rotation.x);
    const float sx = std::sin(rotation.x);
    const float cy = std::cos(rotation.y);
    const float sy = std::sin(rotation.y);
    const float cz = std::cos(rotation.z);
    const float sz = std::sin(rotation.z);

    r::Vec3f result = v;

    result = {result.x, cx * result.y - sx * result.z, sx * result.y + cx * result.z}; ///<< x
    result = {cy * result.x + sy * result.z, result.y, -sy * result.x + cy * result.z};///<< y
    result = {cz * result.x - sz * result.y, sz * result.x + cz * result.y, result.z}; ///<< z

    return result;
}

r::GlobalTransform3d r::GlobalTransform3d::from_local_and_parent(const Transform3d &local, const GlobalTransform3d &parent) noexcept
{
    GlobalTransform3d result;

    result.position = parent.position + _apply_rotation(local.position * parent.scale, parent.rotation);
    result.rotation = parent.rotation + local.rotation;
    result.scale = parent.scale * local.scale;

    return result;
}
