#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

struct Player final {
        Player() = default;
        int hp{100};
};

struct ForceModule final {
        ForceModule() = default;
        r::Vec3f force{0.f, 10.f, 0.f};
};

// clang-format off

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{})
        .add_systems<[](r::ecs::Commands &commands)
        {
            commands.spawn((
                Player{},
                r::Transform3d{.position = {100.f, 200.f, 0.f}}
            )).with_children([&](r::ecs::ChildBuilder &parent)
            {
                parent.spawn((ForceModule{}, r::Transform3d{.position = {50.f, 0.f, 0.f}}));
            });
        }>
        (r::Schedule::STARTUP)
        .run();
}
