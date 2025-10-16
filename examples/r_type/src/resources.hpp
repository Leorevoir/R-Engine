#pragma once

/* ================================================================================= */
/* Constants & Configuration */
/* ================================================================================= */

static constexpr float ENEMY_SPAWN_INTERVAL = 0.75f; /* in seconds */
static constexpr float BOSS_SPAWN_TIME = 20.0f;

/* ================================================================================= */
/* Resources */
/* Resources are global data structures. */
/* ================================================================================= */

struct EnemySpawnTimer {
        float time_left = ENEMY_SPAWN_INTERVAL;
};

struct BossSpawnTimer {
        float time_left = BOSS_SPAWN_TIME;
        bool spawned = false;
};

struct BossShootTimer {
        float time_left = 2.0f;
        static constexpr float FIRE_RATE = 2.0f;
};
