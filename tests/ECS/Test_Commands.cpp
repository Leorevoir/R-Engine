#include "../Test.hpp"

#include "R-Engine/ECS/Command.hpp"
#include "R-Engine/ECS/Scene.hpp"

struct TestPosition {
        f32 x = 0;
        f32 y = 0;
        f32 z = 0;
};

Test(Commands, CommandSpawn)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto commands = r::ecs::Commands(buffer.get());

    // Spawning returns an EntityCommands handle with a placeholder ID.
    auto entity_cmd = commands.spawn();
    const auto placeholder_entity = entity_cmd.id();

    // The entity does not exist in the scene yet.
    cr_assert(placeholder_entity != 0, "Placeholder entity should not be 0.");

    buffer->apply(*scene);

    // The placeholder map in the scene should now map the placeholder to a real entity.
    const auto &placeholder_map = scene->get_command_buffer_placeholder_map();
    cr_assert(placeholder_map.count(placeholder_entity) > 0, "Placeholder should be mapped to a real entity after apply.");
    const auto real_entity = placeholder_map.at(placeholder_entity);
    cr_assert(real_entity == 1, "The first real entity ID should be 1.");
}

Test(Commands, CommandAddComponent)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();
    auto commands = r::ecs::Commands(buffer.get());

    // Chain commands using the EntityCommands builder pattern.
    const auto placeholder_entity = commands.spawn().insert(TestPosition{1.0f, 2.0f, 3.0f}).id();

    // Before applying, the component does not exist.
    cr_assert(scene->has_component<TestPosition>(1) == false, "Component should not exist before applying buffer.");

    buffer->apply(*scene);

    // After applying, the component should exist on the real entity.
    const auto real_entity = scene->get_command_buffer_placeholder_map().at(placeholder_entity);
    cr_assert(scene->has_component<TestPosition>(real_entity) == true, "Component should exist after applying buffer.");
    cr_assert(scene->get_component_ptr<TestPosition>(real_entity)->x == 1.0f);
    cr_assert(scene->get_component_ptr<TestPosition>(real_entity)->y == 2.0f);
    cr_assert(scene->get_component_ptr<TestPosition>(real_entity)->z == 3.0f);
}

Test(Commands, CommandDespawn)
{
    auto scene = std::make_unique<r::ecs::Scene>();
    auto buffer = std::make_unique<r::ecs::CommandBuffer>();

    // First, create an entity directly in the scene to test despawning it.
    const auto entity_to_despawn = scene->create_entity();
    scene->add_component<TestPosition>(entity_to_despawn, {});
    cr_assert(scene->has_component<TestPosition>(entity_to_despawn) == true, "Setup failed: component not added.");

    auto commands = r::ecs::Commands(buffer.get());
    commands.despawn(entity_to_despawn);

    // It should still exist before applying the buffer.
    cr_assert(scene->has_component<TestPosition>(entity_to_despawn) == true, "Entity should not be despawned before apply.");

    buffer->apply(*scene);

    // Now it should be gone.
    cr_assert(scene->has_component<TestPosition>(entity_to_despawn) == false, "Entity should be despawned after apply.");
}

Test(Commands, CommandSpawnNoBuffer)
{
    // A null command object should not crash and return a null entity handle.
    auto commands = r::ecs::Commands();
    auto entity_cmd = commands.spawn();
    cr_assert(entity_cmd.id() == 0, "Spawning with a null buffer should yield entity 0.");
}

Test(Commands, CommandAddComponentNoBuffer)
{
    // A null command object should not crash when adding components.
    auto commands = r::ecs::Commands();
    auto entity_cmd = commands.spawn();
    cr_assert(entity_cmd.id() == 0);

    entity_cmd.insert(TestPosition{1.0f, 2.0f, 3.0f});
    cr_assert(true); // Test passes if it doesn't crash.
}
