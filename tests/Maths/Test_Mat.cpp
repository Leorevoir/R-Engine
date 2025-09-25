#include "../Test.hpp"

#include <R-Engine/Maths/Mat.hpp>
#include <R-Engine/Maths/Vec.hpp>
#include <ostream>

Test(Mat, default_constructor)
{
    r::Mat4f m;

    for (usize i = 0; i < 16; ++i) {
        cr_expect_eq(m.data[i], 0.0f, "Default constructor should init to 0");
    }
}

Test(Mat, value_constructor)
{
    constexpr r::Mat4f m(5.0f);

    for (usize i = 0; i < 16; ++i) {
        cr_expect_eq(m.data[i], 5.0f, "Value constructor failed");
    }
}

Test(Mat, initializer_list)
{
    constexpr r::Mat2f m{1.0f, 2.0f, 3.0f, 4.0f};

    cr_expect_eq(m(0, 0), 1.0f);
    cr_expect_eq(m(0, 1), 2.0f);
    cr_expect_eq(m(1, 0), 3.0f);
    cr_expect_eq(m(1, 1), 4.0f);
}

Test(Mat, identity)
{
    constexpr r::Mat4f identity = r::Mat4f::identity();

    for (usize i = 0; i < 4; ++i) {
        for (usize j = 0; j < 4; ++j) {
            if (i == j) {
                cr_expect_eq(identity(i, j), 1.0f, "Identity diagonal should be 1");
            } else {
                cr_expect_eq(identity(i, j), 0.0f, "Identity off-diagonal should be 0");
            }
        }
    }
}

Test(Mat, access_operator)
{
    r::Mat2f m{1.0f, 2.0f, 3.0f, 4.0f};

    cr_expect_eq(m(0, 0), 1.0f);
    cr_expect_eq(m(0, 1), 2.0f);
    cr_expect_eq(m(1, 0), 3.0f);
    cr_expect_eq(m(1, 1), 4.0f);

    m(0, 1) = 5.0f;
    cr_expect_eq(m(0, 1), 5.0f);
    cr_expect_eq(m.data[1], 5.0f);
}

Test(Mat, transpose)
{
    constexpr r::Mat2f m{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Mat2f transposed = m.transpose();

    cr_expect_eq(transposed(0, 0), 1.0f);
    cr_expect_eq(transposed(0, 1), 3.0f);
    cr_expect_eq(transposed(1, 0), 2.0f);
    cr_expect_eq(transposed(1, 1), 4.0f);
}

Test(Mat, matrix_multiplication)
{
    constexpr r::Mat2f a{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Mat2f b{5.0f, 6.0f, 7.0f, 8.0f};
    constexpr r::Mat2f result = a * b;

    cr_expect_eq(result(0, 0), 19.0f);

    cr_expect_eq(result(0, 1), 22.0f);

    cr_expect_eq(result(1, 0), 43.0f);

    cr_expect_eq(result(1, 1), 50.0f);
}

Test(Mat, vector_multiplication)
{
    constexpr r::Mat2f m{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Vec2f v{5.0f, 6.0f};
    constexpr r::Vec2f result = m * v;

    cr_expect_eq(result.x, 17.0f);

    cr_expect_eq(result.y, 39.0f);
}

Test(Mat, equality_operator)
{
    constexpr r::Mat2f a{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Mat2f b{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Mat2f c{2.0f, 3.0f, 4.0f, 5.0f};

    cr_expect(a == b);
    cr_expect(a != c);
}

Test(Mat, translation_matrix)
{
    constexpr r::Vec3f t{2.0f, 3.0f, 4.0f};
    constexpr r::Mat4f trans_mat = r::translation(t);

    cr_expect_eq(trans_mat(0, 0), 1.0f);
    cr_expect_eq(trans_mat(1, 1), 1.0f);
    cr_expect_eq(trans_mat(2, 2), 1.0f);
    cr_expect_eq(trans_mat(3, 3), 1.0f);
    cr_expect_eq(trans_mat(0, 3), 2.0f);
    cr_expect_eq(trans_mat(1, 3), 3.0f);
    cr_expect_eq(trans_mat(2, 3), 4.0f);

    constexpr r::Vec4f point{1.0f, 1.0f, 1.0f, 1.0f};
    constexpr r::Vec4f translated = trans_mat * point;

    cr_expect_eq(translated.x, 3.0f);
    cr_expect_eq(translated.y, 4.0f);
    cr_expect_eq(translated.z, 5.0f);
    cr_expect_eq(translated.w, 1.0f);
}

Test(Mat, scale_matrix)
{
    constexpr r::Vec3f s{2.0f, 3.0f, 4.0f};
    constexpr r::Mat4f scale_mat = r::scale(s);

    cr_expect_eq(scale_mat(0, 0), 2.0f);
    cr_expect_eq(scale_mat(1, 1), 3.0f);
    cr_expect_eq(scale_mat(2, 2), 4.0f);
    cr_expect_eq(scale_mat(3, 3), 1.0f);

    constexpr r::Vec4f point{1.0f, 2.0f, 3.0f, 1.0f};
    constexpr r::Vec4f scaled = scale_mat * point;

    cr_expect_eq(scaled.x, 2.0f);
    cr_expect_eq(scaled.y, 6.0f);
    cr_expect_eq(scaled.z, 12.0f);
    cr_expect_eq(scaled.w, 1.0f);
}

std::ostream &operator<<(std::ostream &os, const r::Mat4f &mat)
{
    for (usize r = 0; r < 4; ++r) {
        os << "| ";
        for (usize c = 0; c < 4; ++c) {
            os << mat(r, c) << " ";
        }
        os << "|\n";
    }
    return os;
}

Test(Mat, rotation_x_matrix)
{
    constexpr f32 angle = M_PI / 2.0f;
    const r::Mat4f rot_mat = r::rotation_x(angle);

    cr_expect_float_eq(rot_mat(0, 0), 1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(1, 1), 0.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(1, 2), -1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(2, 1), 1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(2, 2), 0.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(3, 3), 1.0f, 1e-6f);

    constexpr r::Vec4f point{0.0f, 1.0f, 0.0f, 1.0f};
    const r::Vec4f rotated = rot_mat * point;

    cr_expect_float_eq(rotated.x, 0.0f, 1e-6f);
    cr_expect_float_eq(rotated.y, 0.0f, 1e-6f);
    cr_expect_float_eq(rotated.z, 1.0f, 1e-6f);
    cr_expect_float_eq(rotated.w, 1.0f, 1e-6f);
}

Test(Mat, rotation_y_matrix)
{
    constexpr f32 angle = M_PI / 2.0f;
    const r::Mat4f rot_mat = r::rotation_y(angle);

    cr_expect_float_eq(rot_mat(0, 0), 0.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(0, 2), 1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(1, 1), 1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(2, 0), -1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(2, 2), 0.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(3, 3), 1.0f, 1e-6f);

    constexpr r::Vec4f point{0.0f, 0.0f, 1.0f, 1.0f};
    const r::Vec4f rotated = rot_mat * point;

    cr_expect_float_eq(rotated.x, 1.0f, 1e-6f);
    cr_expect_float_eq(rotated.y, 0.0f, 1e-6f);
    cr_expect_float_eq(rotated.z, 0.0f, 1e-6f);
    cr_expect_float_eq(rotated.w, 1.0f, 1e-6f);
}

Test(Mat, rotation_z_matrix)
{
    constexpr f32 angle = M_PI / 2.0f;
    const r::Mat4f rot_mat = r::rotation_z(angle);

    cr_expect_float_eq(rot_mat(0, 0), 0.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(0, 1), -1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(1, 0), 1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(1, 1), 0.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(2, 2), 1.0f, 1e-6f);
    cr_expect_float_eq(rot_mat(3, 3), 1.0f, 1e-6f);

    constexpr r::Vec4f point{1.0f, 0.0f, 0.0f, 1.0f};
    const r::Vec4f rotated = rot_mat * point;

    cr_expect_float_eq(rotated.x, 0.0f, 1e-6f);
    cr_expect_float_eq(rotated.y, 1.0f, 1e-6f);
    cr_expect_float_eq(rotated.z, 0.0f, 1e-6f);
    cr_expect_float_eq(rotated.w, 1.0f, 1e-6f);
}

Test(Mat, composition)
{

    constexpr r::Mat4f translate = r::translation(r::Vec3f{1.0f, 2.0f, 3.0f});
    constexpr r::Mat4f scale_mat = r::scale(r::Vec3f{2.0f, 2.0f, 2.0f});
    const r::Mat4f rotate = r::rotation_z(M_PI / 2.0f);

    const r::Mat4f composite = rotate * scale_mat * translate;

    constexpr r::Vec4f point{1.0f, 1.0f, 1.0f, 1.0f};
    const r::Vec4f transformed = composite * point;

    cr_expect_float_eq(transformed.w, 1.0f, 1e-6f);
}

Test(Mat, zero_matrix)
{
    constexpr r::Mat4f zero(0.0f);
    constexpr r::Mat4f identity = r::Mat4f::identity();
    constexpr r::Mat4f result = zero * identity;

    for (usize i = 0; i < 16; ++i) {
        cr_expect_eq(result.data[i], 0.0f);
    }
}

Test(Mat, self_multiply)
{
    constexpr r::Mat2f m{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Mat2f result = m * m;

    cr_expect_eq(result(0, 0), 7.0f);
    cr_expect_eq(result(0, 1), 10.0f);
    cr_expect_eq(result(1, 0), 15.0f);
    cr_expect_eq(result(1, 1), 22.0f);
}

Test(Mat, transpose_identity)
{
    constexpr r::Mat4f identity = r::Mat4f::identity();
    constexpr r::Mat4f transposed = identity.transpose();

    cr_expect(identity == transposed);
}

Test(Mat, mat3_operations)
{
    constexpr r::Mat3f m{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    constexpr r::Mat3f transposed = m.transpose();

    cr_expect_eq(transposed(0, 0), 1.0f);
    cr_expect_eq(transposed(0, 1), 4.0f);
    cr_expect_eq(transposed(0, 2), 7.0f);
    cr_expect_eq(transposed(1, 0), 2.0f);
    cr_expect_eq(transposed(1, 1), 5.0f);
    cr_expect_eq(transposed(1, 2), 8.0f);
    cr_expect_eq(transposed(2, 0), 3.0f);
    cr_expect_eq(transposed(2, 1), 6.0f);
    cr_expect_eq(transposed(2, 2), 9.0f);
}

Test(Mat, equality_floating_point)
{
    constexpr r::Mat2f a{1.0f, 2.0f, 3.0f, 4.0f};
    constexpr r::Mat2f b{1.0f + 1e-7f, 2.0f, 3.0f, 4.0f};

    cr_expect(a == b);
}
