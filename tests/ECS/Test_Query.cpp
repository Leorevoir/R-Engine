#include "../Test.hpp"

#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/ECS/Query.hpp"

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
    const auto command = r::ecs::Commands(scene.get());

    const auto entity = command.spawn();
    command.add_component<Position>(entity, Position{1.0f, 2.0f});

    r::ecs::Query<r::ecs::Ref<Position>> query(scene.get(), {entity});

    for (auto [pos] : query) {
        cr_assert_eq(pos.ptr->x, 1.0f);
        cr_assert_eq(pos.ptr->y, 2.0f);
    }
}

Test(Query, QueryMutAndRef)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    const auto command = r::ecs::Commands(scene.get());

    const auto entity = command.spawn();
    command.add_component<Position>(entity, Position{1.0f, 2.0f});
    command.add_component<Velocity>(entity, Velocity{0.5f, 1.5f});

    r::ecs::Query<r::ecs::Mut<Position>, r::ecs::Ref<Velocity>> query(scene.get(), {entity});

    for (auto [pos, vel] : query) {
        /** @brief read */
        cr_assert_eq(vel.ptr->vx, 0.5f);
        cr_assert_eq(vel.ptr->vy, 1.5f);

        /** @brief mutate */
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
    const auto command = r::ecs::Commands(scene.get());

    const auto e1 = command.spawn();
    const auto e2 = command.spawn();
    command.add_component<Position>(e1, Position{1.0f, 1.0f});
    command.add_component<Position>(e2, Position{2.0f, 2.0f});

    r::ecs::Query<r::ecs::Mut<Position>> query(scene.get(), {e1, e2});

    int i = 0;
    for (auto [pos] : query) {
        pos.ptr->x += 10.0f;
        pos.ptr->y += 20.0f;

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
