#include "../Test.hpp"

#include <R-Engine/Assets/Assets.hpp>

struct DummyAsset {
        i32 value = 0;
};

Test(AssetsTest, AddGetRemove)
{
    r::Assets<DummyAsset> assets;

    auto h1 = assets.add(std::make_unique<DummyAsset>(DummyAsset{42}));
    cr_assert(h1.valid(), "Added asset handle should be valid");
    auto *ptr = assets.get(h1);
    cr_assert(ptr != nullptr, "Get should return non-null poi32er");
    cr_assert_eq(ptr->value, 42, "Value should match inserted asset");

    auto h2 = assets.add(DummyAsset{99});
    auto *ptr2 = assets.get(h2);
    cr_assert(ptr2 != nullptr, "Get by value-added asset should not be null");
    cr_assert_eq(ptr2->value, 99, "Value should match");

    assets.remove(h1);
    cr_assert_null(assets.get(h1), "Removed asset should return nullptr");
    cr_assert(ptr2 == assets.get(h2), "Other asset should remain i32act");
}

Test(AssetsTest, ForEachIteration)
{
    r::Assets<DummyAsset> assets;
    auto h1 = assets.add(DummyAsset{1});
    auto h2 = assets.add(DummyAsset{2});
    i32 sum = 0;

    assets.for_each([&](u32 id, DummyAsset *asset) { sum += asset->value; });

    cr_assert_eq(sum, 3, "Sum of asset values should be 1 + 2 = 3");
}

Test(AssetsTest, InvalidHandle)
{
    r::Assets<DummyAsset> assets;
    r::assets::Handle<DummyAsset> invalidHandle;

    cr_assert(!invalidHandle.valid(), "Default constructed handle should be invalid");
    cr_assert_null(assets.get(invalidHandle), "Getting asset with invalid handle should return nullptr");
}

Test(AssetsTest, RemoveNonExistent)
{
    r::Assets<DummyAsset> assets;
    r::assets::Handle<DummyAsset> nonExistentHandle(999);

    assets.remove(nonExistentHandle);
    cr_assert(true, "Removing non-existent handle should not crash");
}

Test(AssetsTest, RemoveExistent)
{
    r::Assets<DummyAsset> assets;
    auto h1 = assets.add(DummyAsset{10});

    cr_assert(assets.get(h1) != nullptr, "Asset should exist before removal");
    assets.remove(h1);
    cr_assert(assets.get(h1) == nullptr, "Asset should be null after removal");
}
