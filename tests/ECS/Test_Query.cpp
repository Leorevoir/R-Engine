#include "../Test.hpp"

#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/ECS/Query.hpp"
#include "R-Engine/ECS/Scene.hpp"

struct Position {
        float x = 0;
        float y = 0;
};

struct Velocity {
        float vx = 0;
        float vy = 0;
};

struct Health {
        int value = 100;
};

Test(Query, QuerySingleComponent)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto commands = r::ecs::Commands(buffer.get());

    commands.spawn(Position{1.0f, 2.0f});
    buffer->apply(*scene);

    const r::ecs::Entity entity = 1;
    r::ecs::Query<r::ecs::Ref<Position>> query(scene.get(), {entity});

    for (auto [pos] : query) {
        cr_assert_eq(pos.ptr->x, 1.0f);
        cr_assert_eq(pos.ptr->y, 2.0f);
    }
}

Test(Query, QueryMutAndRef)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto commands = r::ecs::Commands(buffer.get());

    commands.spawn(Position{1.0f, 2.0f}, Velocity{0.5f, 1.5f});
    buffer->apply(*scene);

    const r::ecs::Entity entity = 1;
    r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>> query(scene.get(), {entity});

    for (auto [pos, vel] : query) {
        cr_assert_eq(vel.ptr->vx, 0.5f);
        cr_assert_eq(vel.ptr->vy, 1.5f);

        pos.ptr->x += vel.ptr->vx;
        pos.ptr->y += vel.ptr->vy;
    }

    const auto pos_ptr = scene->get_component_ptr<Position>(entity);

    cr_assert_eq(pos_ptr->x, 1.5f);
    cr_assert_eq(pos_ptr->y, 3.5f);
}

Test(Query, QueryMultipleEntities)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto commands = r::ecs::Commands(buffer.get());

    commands.spawn(Position{1.0f, 1.0f});
    commands.spawn(Position{2.0f, 2.0f});
    buffer->apply(*scene);

    const r::ecs::Entity e1 = 1, e2 = 2;
    r::ecs::Query<r::ecs::Mut<Position>> query(scene.get(), {e1, e2});

    int i = 0;
    for (auto [pos] : query) {
        pos.ptr->x += 10.0f;
        pos.ptr->y += 20.0f;

        // Note: The order of iteration is not guaranteed in a real ECS.
        // For this simple test, we assume the insertion order is maintained.
        // A more robust test would check for values without relying on order.
        if (i == 0) {
            cr_assert_eq(pos.ptr->x, 11.0f);
            cr_assert_eq(pos.ptr->y, 21.0f);
        } else {
            cr_assert_eq(pos.ptr->x, 12.0f);
            cr_assert_eq(pos.ptr->y, 22.0f);
        }
        ++i;
    }
}

Test(Query, QueryEmptyEntities)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    r::ecs::Query<r::ecs::Ref<Position>> query(scene.get(), {});

    int count = 0;
    for (auto [pos] : query) {
        (void) pos;
        ++count;
    }
    cr_assert_eq(count, 0);
}
