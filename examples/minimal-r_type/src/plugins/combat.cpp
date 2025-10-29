#include "combat.hpp"
#include "../components.hpp"
#include "../resources.hpp"
#include "../state.hpp"

#include "R-Engine/Components/Transform3d.hpp"
#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/RunConditions.hpp>
#include <iostream>
#include <vector>

/* ================================================================================= */
/* Combat Systems */
/* ================================================================================= */

static void collision_system(r::ecs::Commands &commands,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<PlayerBullet>> bullet_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Enemy>> enemy_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::Mut<Health>, r::ecs::With<Boss>> boss_query)
{
    std::vector<r::ecs::Entity> despawn_queue;

    for (auto bullet_it = bullet_query.begin(); bullet_it != bullet_query.end(); ++bullet_it) {
        auto [bullet_transform, bullet_collider, _b] = *bullet_it;
        bool bullet_collided = false;

        /* Collision with enemies */
        for (auto enemy_it = enemy_query.begin(); enemy_it != enemy_query.end(); ++enemy_it) {
            auto [enemy_transform, enemy_collider, _e] = *enemy_it;
            float distance = (bullet_transform.ptr->position - enemy_transform.ptr->position).length();
            float radii_sum = bullet_collider.ptr->radius + enemy_collider.ptr->radius;

            if (distance < radii_sum) {
                despawn_queue.push_back(enemy_it.entity());
                bullet_collided = true;
                break;
            }
        }

        if (bullet_collided) {
            despawn_queue.push_back(bullet_it.entity());
            continue;
        }

        /* Collision with boss */
        for (auto boss_it = boss_query.begin(); boss_it != boss_query.end(); ++boss_it) {
            auto [boss_transform, boss_collider, health, _boss] = *boss_it;
            float distance = (bullet_transform.ptr->position - boss_transform.ptr->position).length();
            float radii_sum = bullet_collider.ptr->radius + boss_collider.ptr->radius;

            if (distance < radii_sum) {
                despawn_queue.push_back(bullet_it.entity());
                health.ptr->current -= 10;
                std::cout << "Boss hit! HP: " << health.ptr->current << "/" << health.ptr->max << std::endl;

                if (health.ptr->current <= 0) {
                    despawn_queue.push_back(boss_it.entity());
                    r::Logger::info("Boss defeated!");
                }
                break;
            }
        }
    }

    for (r::ecs::Entity entity : despawn_queue) {
        commands.despawn(entity);
    }
}

static void player_collision_system(r::ecs::ResMut<r::NextState<GameState>> next_state,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Player>> player_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Enemy>> enemy_query)
{
    for (auto [player_transform, player_collider, _p] : player_query) {
        for (auto [enemy_transform, enemy_collider, _e] : enemy_query) {
            float distance = (player_transform.ptr->position - enemy_transform.ptr->position).length();
            float sum_radii = player_collider.ptr->radius + enemy_collider.ptr->radius;

            if (distance < sum_radii) {
                r::Logger::warn("Player collision! Game Over.");
                next_state.ptr->set(GameState::GameOver);
                return;
            }
        }
    }
}

static void player_bullet_collision_system(r::ecs::ResMut<r::NextState<GameState>> next_state,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<Player>> player_query,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Ref<Collider>, r::ecs::With<EnemyBullet>> bullet_query)
{
    for (auto [player_transform, player_collider, _p] : player_query) {
        for (auto [bullet_transform, bullet_collider, _b] : bullet_query) {
            float distance = (player_transform.ptr->position - bullet_transform.ptr->position).length();
            float sum_radii = player_collider.ptr->radius + bullet_collider.ptr->radius;

            if (distance < sum_radii) {
                r::Logger::warn("Player hit by bullet! Game Over.");
                next_state.ptr->set(GameState::GameOver);
                return;
            }
        }
    }
}

static void despawn_offscreen_system(r::ecs::Commands &commands,
    r::ecs::Query<r::ecs::Ref<r::Transform3d>, r::ecs::Without<Player>, r::ecs::Without<Boss>> query)
{
    const float despawn_boundary_x = 100.0f;
    for (auto it = query.begin(); it != query.end(); ++it) {
        auto [transform, _, __] = *it;
        if (std::abs(transform.ptr->position.x) > despawn_boundary_x) {
            commands.despawn(it.entity());
        }
    }
}

template<typename T>
static void despawn_all_entities_with(r::ecs::Commands &commands, r::ecs::Query<r::ecs::With<T>> &query)
{
    for (auto it = query.begin(); it != query.end(); ++it) {
        commands.despawn(it.entity());
    }
}

static void cleanup_battle_system(r::ecs::Commands &commands, r::ecs::ResMut<EnemySpawnTimer> spawn_timer,
    r::ecs::ResMut<BossSpawnTimer> boss_spawn_timer, r::ecs::Query<r::ecs::With<Enemy>> enemy_query,
    r::ecs::Query<r::ecs::With<PlayerBullet>> player_bullet_query, r::ecs::Query<r::ecs::With<EnemyBullet>> enemy_bullet_query,
    r::ecs::Query<r::ecs::With<Player>> player_query, r::ecs::Query<r::ecs::With<Boss>> boss_query)
{
    despawn_all_entities_with<Enemy>(commands, enemy_query);
    despawn_all_entities_with<PlayerBullet>(commands, player_bullet_query);
    despawn_all_entities_with<EnemyBullet>(commands, enemy_bullet_query);
    despawn_all_entities_with<Player>(commands, player_query);
    despawn_all_entities_with<Boss>(commands, boss_query);

    spawn_timer.ptr->time_left = ENEMY_SPAWN_INTERVAL;
    boss_spawn_timer.ptr->time_left = BOSS_SPAWN_TIME;
    boss_spawn_timer.ptr->spawned = false;
}

void CombatPlugin::build(r::Application &app)
{
    app.add_systems<cleanup_battle_system>(r::OnEnter{GameState::EnemiesBattle})

        .add_systems<despawn_offscreen_system>(r::Schedule::UPDATE)

        .add_systems<collision_system, player_collision_system, player_bullet_collision_system>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<GameState::EnemiesBattle>>()
        .run_or<r::run_conditions::in_state<GameState::BossBattle>>();
}
