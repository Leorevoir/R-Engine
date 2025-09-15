#include "../Test.hpp"

#include <R-Engine/Maths/Vec.hpp>

using namespace r;

Test(Vec, default_constructor)
{
    Vec3f v;

    for (usize i = 0; i < v.size; ++i) {
        cr_expect_eq(v.data[i], 0.0f, "Default constructor should init to 0");
    }
}

Test(Vec, value_constructor)
{
    constexpr Vec3f v(5.0f);

    for (usize i = 0; i < v.size; ++i) {
        cr_expect_eq(v.data[i], 5.0f, "Value constructor failed");
    }
}

Test(Vec, initializer_list)
{
    constexpr Vec3i v{1, 2, 3};
    cr_expect_eq(v.x, 1);
    cr_expect_eq(v.y, 2);
    cr_expect_eq(v.z, 3);
}

Test(Vec, addition_operator)
{
    constexpr Vec3i a{1, 2, 3};
    constexpr Vec3i b{4, 5, 6};
    constexpr Vec3i c = a + b;

    cr_expect_eq(c.x, 5);
    cr_expect_eq(c.y, 7);
    cr_expect_eq(c.z, 9);
}

Test(Vec, scalar_multiplication)
{
    constexpr Vec2f v{1.f, -2.f};
    constexpr Vec2f w = v * 3.f;

    cr_expect_float_eq(w.x, 3.f, 1e-6);
    cr_expect_float_eq(w.y, -6.f, 1e-6);
}

Test(Vec, dot_product)
{
    constexpr Vec3i a{1, 2, 3};
    constexpr Vec3i b{4, 5, 6};
    int dot = a.dot(b);

    cr_expect_eq(dot, 32, "Dot product incorrect (expected 32, got %d)", dot);
}

Test(Vec, cross_product)
{
    constexpr Vec3i a{1, 0, 0};
    constexpr Vec3i b{0, 1, 0};
    const Vec3i c = a.cross(b);

    cr_expect_eq(c.x, 0);
    cr_expect_eq(c.y, 0);
    cr_expect_eq(c.z, 1);
}

Test(Vec, normalize)
{
    constexpr Vec2f v{3.f, 4.f};
    const Vec2f n = v.normalize();

    cr_expect_float_eq(n.length(), 1.f, 1e-6, "Normalized vector should have length 1");
}

Test(Vec, equality_operator)
{
    constexpr Vec2i a{1, 2};
    constexpr Vec2i b{1, 2};
    constexpr Vec2i c{2, 3};

    cr_expect(a == b);
    cr_expect(a != c);
}
