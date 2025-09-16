#include "../Test.hpp"

#include "R-Engine/ECS/Command.hpp"

Test(Commands, CommandSpawn)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    const auto command = r::ecs::Commands(scene.get());
    const auto entity = command.spawn();

    cr_assert(entity != 0);
    cr_assert_eq(entity, 1);

    const auto entity_2 = command.spawn();

    cr_assert(entity_2 != 0);
}

struct TestPosition {
        f32 x = 0;
        f32 y = 0;
        f32 z = 0;
};

Test(Commands, CommandAddComponent)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    const auto command = r::ecs::Commands(scene.get());
    const auto entity = command.spawn();

    cr_assert(entity != 0);
    cr_assert_eq(entity, 1);

    command.add_component<TestPosition>(entity, TestPosition{1.0f, 2.0f, 3.0f});

    cr_assert(scene->has_component<TestPosition>(entity) == true);
    cr_assert(scene->get_component_ptr<TestPosition>(entity)->x == 1.0f);
    cr_assert(scene->get_component_ptr<TestPosition>(entity)->y == 2.0f);
    cr_assert(scene->get_component_ptr<TestPosition>(entity)->z == 3.0f);
}

Test(Commands, CommandSpawnNoScene)
{
    const auto command = r::ecs::Commands();
    const auto entity = command.spawn();

    cr_assert(entity == 0);
}

Test(Commands, CommandAddComponentNoScene)
{
    const auto command = r::ecs::Commands();
    const auto entity = command.spawn();
    cr_assert(entity == 0);

    command.add_component<TestPosition>(entity, TestPosition{1.0f, 2.0f, 3.0f});
    cr_assert(true);
}
