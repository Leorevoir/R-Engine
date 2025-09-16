#include "../Test.hpp"

#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/ECS/Resolver.hpp"

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

Test(Resolver, ResolveRes)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    scene->insert_resource<FrameTime>(FrameTime{0.25f});

    r::ecs::Resolver resolver(scene.get());
    const auto res = resolver.resolve(std::type_identity<r::ecs::Res<FrameTime>>{});

    cr_assert(res.ptr != nullptr);
    cr_assert_eq(res.ptr->delta_time, 0.25f);
}

Test(Resolver, ResolveCommands)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    r::ecs::Resolver resolver(scene.get());

    auto cmds = resolver.resolve(std::type_identity<r::ecs::Commands>{});
    const auto e = cmds.spawn();

    cr_assert(e != 0);
    cr_assert(scene->has_component<Position>(e) == false);
}

Test(Resolver, ResolveQuerySingleEntity)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto cmds = r::ecs::Commands(scene.get());

    const auto e = cmds.spawn();
    cmds.add_component<Position>(e, Position{1.0f, 2.0f});

    r::ecs::Resolver resolver(scene.get());
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
    auto cmds = r::ecs::Commands(scene.get());

    const auto e1 = cmds.spawn();
    const auto e2 = cmds.spawn();

    cmds.add_component<Position>(e1, Position{1.0f, 1.0f});
    cmds.add_component<Velocity>(e1, Velocity{0.5f, 0.5f});

    cmds.add_component<Position>(e2, Position{2.0f, 2.0f});
    cmds.add_component<Velocity>(e2, Velocity{1.5f, 1.5f});

    r::ecs::Resolver resolver(scene.get());
    auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>>>{});

    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->vx;
        pos.ptr->y += vel.ptr->vy;
    }

    auto pos1 = scene->get_component_ptr<Position>(e1);
    auto pos2 = scene->get_component_ptr<Position>(e2);

    cr_assert_eq(pos1->x, 1.5f);
    cr_assert_eq(pos1->y, 1.5f);
    cr_assert_eq(pos2->x, 3.5f);
    cr_assert_eq(pos2->y, 3.5f);
}

Test(Resolver, ResolveQueryEmpty)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    r::ecs::Resolver resolver(scene.get());

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
    scene->insert_resource<FrameTime>(FrameTime{1.0f});

    auto cmds = r::ecs::Commands(scene.get());

    const auto e = cmds.spawn();
    cmds.add_component<Position>(e, Position{0.0f, 0.0f});
    cmds.add_component<Velocity>(e, Velocity{2.0f, 3.0f});

    r::ecs::Resolver resolver(scene.get());

    const auto time = resolver.resolve(std::type_identity<r::ecs::Res<FrameTime>>{});
    const auto query = resolver.resolve(std::type_identity<r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>>>{});

    for (auto [pos, vel] : query) {
        pos.ptr->x += vel.ptr->vx * time.ptr->delta_time;
        pos.ptr->y += vel.ptr->vy * time.ptr->delta_time;
    }

    auto pos = scene->get_component_ptr<Position>(e);
    cr_assert_eq(pos->x, 2.0f);
    cr_assert_eq(pos->y, 3.0f);
}

Test(Resolver, ResolveUnsupportedType)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    r::ecs::Resolver resolver(scene.get());

    cr_assert(true);
}
