#pragma once

#include <R-Engine/Maths/Vec.hpp>

/* ================================================================================= */
/* Menu Components */
/* ================================================================================= */

struct MenuButton {
        enum class Action { None, Play, Options, Quit };
        Action action = Action::None;
};

struct MenuRoot {
};
struct GameOverRoot {
};

/* ================================================================================= */
/* Game Components */
/* Components are simple data structures that define the properties of an entity. */
/* ================================================================================= */

struct Player {
};
struct Boss {
};
struct Enemy {
};
struct PlayerBullet {
};
struct EnemyBullet {
};

struct FireCooldown {
        float timer = 0.0f;
};

struct Velocity {
        r::Vec3f value;
};

struct Collider {
        float radius;
};

struct Health {
        int current;
        int max;
};
