#include <R-Engine/Components/Components3d.hpp>

/**
* public
*/

r::Transform3d r::Transform3d::Identity() noexcept
{
    return Transform3d{};
}

r::Mat4f r::Transform3d::to_matrix() const noexcept
{
    const Mat4f mat_scale = r::mat::scale(scale);///<< S

    const Mat4f mat_rot_x = r::mat::rotation_x(rotation.data[0]);
    const Mat4f mat_rot_y = r::mat::rotation_y(rotation.data[1]);
    const Mat4f mat_rot_z = r::mat::rotation_z(rotation.data[2]);
    const Mat4f mat_rot = mat_rot_z * mat_rot_y * mat_rot_x;///<< R

    const Mat4f mat_trans = r::mat::translation(position);///<< T

    /** @info S * R * T */
    const Mat4f model = mat_scale * mat_rot * mat_trans;

    return model;
}
