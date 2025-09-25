#include "../Test.hpp"

#include <R-Engine/Assets/AssetEvents.hpp>

struct DummyAsset {
        i32 value = 0;
};

Test(AssetEventsTest, PushReadClear)
{
    r::assets::AssetEvents<DummyAsset> events;
    r::assets::Handle<DummyAsset> h1(1);
    r::assets::Handle<DummyAsset> h2(2);

    events.push(r::assets::AssetEvent<DummyAsset>::Type::Created, h1);
    events.push(r::assets::AssetEvent<DummyAsset>::Type::Modified, h2);

    const auto &list = events.read();

    cr_assert_eq(list.size(), 2, "Should have 2 events");
    cr_assert_eq(list[0].handle.id(), h1.id(), "First event handle id should match");
    cr_assert_eq(list[1].type, r::assets::AssetEvent<DummyAsset>::Type::Modified, "Second event type should be Modified");

    events.clear();
    cr_assert_eq(events.read().size(), 0, "After clear, event list should be empty");
}

Test(AssetEventsTest, MultiplePush)
{
    r::assets::AssetEvents<DummyAsset> events;
    r::assets::Handle<DummyAsset> h1(1);
    r::assets::Handle<DummyAsset> h2(2);
    r::assets::Handle<DummyAsset> h3(3);

    events.push(r::assets::AssetEvent<DummyAsset>::Type::Created, h1);
    events.push(r::assets::AssetEvent<DummyAsset>::Type::Modified, h2);
    events.push(r::assets::AssetEvent<DummyAsset>::Type::Removed, h3);
    const auto &list = events.read();

    cr_assert_eq(list.size(), 3, "Should have 3 events");
    cr_assert_eq(list[2].handle.id(), h3.id(), "Third event handle id should match");
    cr_assert_eq(list[2].type, r::assets::AssetEvent<DummyAsset>::Type::Removed, "Third event type should be Removed");
}

Test(AssetEventsTest, NoEvents)
{
    r::assets::AssetEvents<DummyAsset> events;
    const auto &list = events.read();

    cr_assert_eq(list.size(), 0, "Newly created AssetEvents should have no events");
}

Test(AssetEventsTest, ClearWithoutPush)
{
    r::assets::AssetEvents<DummyAsset> events;
    events.clear();
    cr_assert_eq(events.read().size(), 0, "After clear, event list should still be empty");
}
