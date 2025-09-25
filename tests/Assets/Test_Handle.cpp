#include "../Test.hpp"

#include <R-Engine/Assets/Handle.hpp>

struct DummyAsset {
        i32 value = 0;
};

Test(HandleTest, DefaultInvalid)
{
    r::assets::Handle<DummyAsset> h;

    cr_assert(!h.valid(), "Default handle should be invalid");
    cr_assert_eq(h.id(), r::assets::Handle<DummyAsset>::InvalidId, "Default handle id should be 0");
}

Test(HandleTest, CustomIdValid)
{
    r::assets::Handle<DummyAsset> h1(5);
    cr_assert(h1.valid(), "r::assets::Handle with id 5 should be valid");
    cr_assert_eq(h1.id(), 5, "r::assets::Handle id should be 5");

    r::assets::Handle<DummyAsset> h2(5);
    cr_assert(h1 == h2, "r::assets::Handles with same id should be equal");
    cr_assert(!(h1 != h2), "r::assets::Handles with same id should not be unequal");

    r::assets::Handle<DummyAsset> h3(6);
    cr_assert(h1 != h3, "r::assets::Handles with different ids should not be equal");
}
