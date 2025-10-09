#include <R-Engine/Components/Transform3d.hpp>
#include <R-Engine/Maths/Quaternion.hpp>

/**
* public
*/

r::GlobalTransform3d r::GlobalTransform3d::from_local_and_parent(const Transform3d &local, const GlobalTransform3d &parent) noexcept
{
    GlobalTransform3d result;

    /* Convert parent and local rotations (Euler angles in radians) to quaternions for correct composition. */
    const Quaternion parent_rotation_q = Quaternion::from_euler(parent.rotation);
    const Quaternion local_rotation_q = Quaternion::from_euler(local.rotation);

    /* Combine the parent's global rotation and the child's local rotation. */
    /* The order (parent * local) applies the local rotation relative to the parent's orientation. */
    const Quaternion global_rotation_q = parent_rotation_q * local_rotation_q;

    /* Convert the resulting quaternion back to Euler angles for storage in the component. */
    result.rotation = global_rotation_q.to_euler();

    /* To calculate the final position:
    1. Scale the local position by the parent's scale.
    2. Rotate this scaled position by the parent's rotation.
    3. Add the result to the parent's global position. */
    const Vec3f scaled_local_position = local.position * parent.scale;
    const Vec3f rotated_local_position = parent_rotation_q.rotate(scaled_local_position);
    result.position = parent.position + rotated_local_position;

    /* Global scale is the component-wise product of parent and local scales. */
    result.scale = parent.scale * local.scale;

    return result;
}
