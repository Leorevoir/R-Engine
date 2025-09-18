#include "../Test.hpp"

#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/ECS/Resolver.hpp"
#include "R-Engine/ECS/Scene.hpp"

struct FrameTime {
        float delta_time = 0.0f;
};

struct Position {
        float x = 0.0f;
        float y = 0.0f;
};

struct Velocity {
        float vx = 0.0f;
        float vy = 0.0f;
};

struct Health {
        int value = 100;
};
struct PlayerTag {
};

Test(Resolver, ResolveRes)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    scene->insert_resource<FrameTime>(FrameTime{0.25f});

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    const auto res = resolver.resolve(std::type_identity<r::ecs::Res<FrameTime>>{});

    cr_assert(res.ptr != nullptr);
    cr_assert_eq(res.ptr->delta_time, 0.25f);
}

Test(Resolver, ResolveCommands)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    r::ecs::Resolver resolver(scene.get(), buffer.get());

    auto cmds = resolver.resolve(std::type_identity<r::ecs::Commands>{});
    const auto placeholder_entity = cmds.spawn().id();

    cr_assert(placeholder_entity != 0);
    // The entity doesn't exist yet, so we can't check its components in the scene.
    // Applying the buffer would make it exist.
    buffer->apply(*scene);
    const auto real_entity = scene->get_command_buffer_placeholder_map().at(placeholder_entity);
    cr_assert(scene->has_component<Position>(real_entity) == false);
}

Test(Resolver, ResolveQuerySingleEntity)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto cmds = r::ecs::Commands(buffer.get());

    cmds.spawn().insert(Position{1.0f, 2.0f});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Ref<Position>>>{});

    int count = 0;
    for (auto [pos] : query) {
        cr_assert_eq(pos.ptr->x, 1.0f);
        cr_assert_eq(pos.ptr->y, 2.0f);
        ++count;
    }
    cr_assert_eq(count, 1);
}

Test(Resolver, ResolveQueryMultipleEntities)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto cmds = r::ecs::Commands(buffer.get());

    cmds.spawn().insert(Position{1.0f, 1.0f}).insert(Velocity{0.5f, 0.5f});
    cmds.spawn().insert(Position{2.0f, 2.0f}).insert(Velocity{1.5f, 1.5f});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>>>{});

    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->vx;
        pos.ptr->y += vel.ptr->vy;
    }

    auto pos1 = scene->get_component_ptr<Position>(1);
    auto pos2 = scene->get_component_ptr<Position>(2);

    cr_assert_eq(pos1->x, 1.5f);
    cr_assert_eq(pos1->y, 1.5f);
    cr_assert_eq(pos2->x, 3.5f);
    cr_assert_eq(pos2->y, 3.5f);
}

Test(Resolver, ResolveQueryEmpty)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    r::ecs::Resolver resolver(scene.get(), buffer.get());

    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Ref<Position>>>{});
    int count = 0;
    for (auto [pos] : query) {
        (void) pos;
        ++count;
    }
    cr_assert_eq(count, 0);
}

Test(Resolver, CombinedResAndQuery)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    scene->insert_resource<FrameTime>(FrameTime{1.0f});

    auto cmds = r::ecs::Commands(buffer.get());
    cmds.spawn().insert(Position{0.0f, 0.0f}).insert(Velocity{2.0f, 3.0f});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    const auto time = resolver.resolve(std::type_identity<r::ecs::Res<FrameTime>>{});
    const auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>>>{});

    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->vx * time.ptr->delta_time;
        pos.ptr->y += vel.ptr->vy * time.ptr->delta_time;
    }

    auto pos = scene->get_component_ptr<Position>(1);
    cr_assert_eq(pos->x, 2.0f);
    cr_assert_eq(pos->y, 3.0f);
}

Test(Resolver, ResolveUnsupportedType)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    r::ecs::Resolver resolver(scene.get(), buffer.get());
    // This test doesn't do anything, but it shouldn't fail to compile.
    // A static_assert would trigger a compile-time error.
    cr_assert(true);
}

Test(Resolver, ResolveQueryWithFilter)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto cmds = r::ecs::Commands(buffer.get());

    cmds.spawn().insert(Position{10, 10}).insert(PlayerTag{});
    cmds.spawn().insert(Position{20, 20});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Ref<Position>, r::ecs::With<PlayerTag>>>{});

    int count = 0;
    for (auto [pos, _] : query) {
        cr_assert_eq(pos.ptr->x, 10, "Should only find entity with PlayerTag");
        count++;
    }
    cr_assert_eq(count, 1, "Expected to find exactly one entity");
}

Test(Resolver, ResolveQueryWithoutFilter)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto cmds = r::ecs::Commands(buffer.get());

    cmds.spawn().insert(Position{10, 10}).insert(Velocity{1, 1});
    cmds.spawn().insert(Position{20, 20});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Ref<Position>, r::ecs::Without<Velocity>>>{});

    int count = 0;
    for (auto [pos, _] : query) {
        cr_assert_eq(pos.ptr->x, 20, "Should only find entity without Velocity");
        count++;
    }
    cr_assert_eq(count, 1, "Expected to find exactly one entity");
}

Test(Resolver, ResolveQueryOptionalFilter)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto cmds = r::ecs::Commands(buffer.get());

    cmds.spawn().insert(Position{10, 10}).insert(Health{80});
    cmds.spawn().insert(Position{20, 20});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Ref<Position>, r::ecs::Optional<Health>>>{});

    int count = 0;
    bool found_e1 = false;
    bool found_e2 = false;

    for (auto [pos, health_opt] : query) {
        if (pos.ptr->x == 10) {
            found_e1 = true;
            cr_assert(health_opt.ptr != nullptr, "Entity with x=10 should have a Health component");
            cr_assert_eq(health_opt.ptr->value, 80);
        } else if (pos.ptr->x == 20) {
            found_e2 = true;
            cr_assert(health_opt.ptr == nullptr, "Entity with x=20 should not have a Health component");
        }
        count++;
    }
    cr_assert_eq(count, 2, "Expected to find two entities");
    cr_assert(found_e1 && found_e2, "Should have found both entities");
}

Test(Resolver, ResolveQueryCombinedFilters)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto cmds = r::ecs::Commands(buffer.get());

    // e1: Player with Position but no Velocity (should be found)
    cmds.spawn().insert(Position{1, 1}).insert(PlayerTag{});
    // e2: Player with Position and Velocity (should be excluded by Without)
    cmds.spawn().insert(Position{2, 2}).insert(PlayerTag{}).insert(Velocity{1, 1});
    // e3: Non-player with Position (should be excluded by With)
    cmds.spawn().insert(Position{3, 3});
    buffer->apply(*scene);

    r::ecs::Resolver resolver(scene.get(), buffer.get());
    auto query =
        resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Ref<Position>, r::ecs::With<PlayerTag>, r::ecs::Without<Velocity>>>{});

    int count = 0;
    for (auto [pos, _, __] : query) {
        cr_assert_eq(pos.ptr->x, 1, "Should only find entity with Position, PlayerTag, and no Velocity");
        count++;
    }
    cr_assert_eq(count, 1, "Expected to find exactly one entity");
}
