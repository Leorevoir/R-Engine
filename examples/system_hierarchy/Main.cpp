#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

#include <iostream>

static constexpr f32 PLAYER_SPEED = 200.f;

struct Player {
};

struct Velocity : public r::Vec3f {
    public:
        Velocity operator=(const r::Vec3f &other)
        {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            return *this;
        }
};

struct Controllable {
};

struct ForceModule {
};

struct DoNotChange {
};

using QPlayer = r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<r::GlobalTransform3d>, r::ecs::With<Player>>;
using QModule = r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<r::GlobalTransform3d>, r::ecs::With<ForceModule>>;
using QDoNotChange = r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::With<DoNotChange>>;

// clang-format off

static void startup_player(r::ecs::Commands &commands)
{
    commands.spawn(
        Controllable{},
        Player{},
        r::Transform3d{.position = {100.f, 200.f, 0.f} },
        Velocity{{.0f, .0f, .0f}}
    ).with_children([&](r::ecs::ChildBuilder &parent)
    {
        parent.spawn(
            ForceModule{}, ///<< ForceModule will be updated by the Player
            r::Transform3d{.position = {50.f, 0.f, 0.f}}
        );
    });

    commands.spawn(
        DoNotChange{}, ///<< as it is not a child of Player, it will not be updated
        r::Transform3d{.position = {400.f, 200.f, 0.f}}
    );
}

static void startup_input(const r::ecs::ResMut<r::InputMap> input_map)
{
    auto *map = input_map.ptr;

    map->bindAction("MoveForward", r::KEYBOARD, KEY_W);
    map->bindAction("MoveBackward", r::KEYBOARD, KEY_S);
}

static void update_player(
    const r::ecs::Res<r::UserInput> user_input,
    const r::ecs::Res<r::InputMap> input_map,
    const r::ecs::Query<r::ecs::Mut<Velocity>, r::ecs::With<Controllable>> query)
{
    const auto *map = input_map.ptr;
    const auto input = *user_input.ptr;

    for (auto [velocity, _]: query) {
        r::Vec3f direction = {.0f, .0f, .0f};

        if (map->isActionPressed("MoveForward", input)) {
            direction.y -= 1.f;
        }
        if (map->isActionPressed("MoveBackward", input)) {
            direction.y += 1.f;
        }

        if (direction.length() > .0f) {
            *velocity.ptr = direction.normalize() * PLAYER_SPEED;
        } else {
            *velocity.ptr = {.0f, .0f, .0f};
        }
    }
}

static std::ostream &operator<<(std::ostream &os, const r::Vec3f &vec)
{
    os << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
    return os;
}

static void update_position(
    const r::ecs::Res<r::core::FrameTime> time,
    const r::ecs::Query<r::ecs::Mut<r::Transform3d>, r::ecs::Ref<Velocity>> query)
{
    for (auto [transform, velocity]: query) {
        transform.ptr->position += *velocity.ptr * time.ptr->delta_time;
    }
}

static void log_transforms(const QPlayer player_query, const QModule module_query, const QDoNotChange do_not_change_query)
{
    for (const auto &[local, global, _] : player_query) {
        if (local.ptr && global.ptr) {
            std::cout << "Player - Local Pos: " << local.ptr->position
                      << " | Global Pos: " << global.ptr->position << std::endl;
        }
    }

    for (const auto &[local, global, _] : module_query) {
         if (local.ptr && global.ptr) {
            std::cout << "Module - Local Pos: " << local.ptr->position
                      << " | Global Pos: " << global.ptr->position << std::endl;
        }
    }

    for (const auto &[local, _] : do_not_change_query) {
         if (local.ptr) {
            std::cout << "DoNotChange - Local Pos: " << local.ptr->position << std::endl;
        }
    }

    std::cout << "--------------------------------" << std::endl;
}

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{})
        .add_systems<startup_player, startup_input>(r::Schedule::STARTUP)
        .add_systems<update_player, update_position>(r::Schedule::UPDATE)
        .add_systems<log_transforms>(r::Schedule::BEFORE_RENDER_2D)
        .run();
}
