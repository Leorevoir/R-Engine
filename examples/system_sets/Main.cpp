#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <iostream>

/* ================================================================================= */
/* Example Objective */
/*                                                                                   */
/* This example demonstrates the "System Sets" feature. We will define several      */
/* groups of systems (Input, Physics, Rendering) and enforce an execution order    */
/* between these entire groups, regardless of the order in which individual        */
/* systems are added to the application.                                           */
/*                                                                                   */
/* Desired execution flow:                                                         */
/* 1. All Input systems run.                                                       */
/* 2. All Physics systems run.                                                     */
/* 3. All Rendering systems run.                                                   */
/*                                                                                   */
/* We will deliberately add the systems to the Application in a "wrong" order      */
/* (Render -> Physics -> Input) to prove that the scheduler correctly reorders      */
/* them based on the set configurations.                                           */
/* ================================================================================= */

/* ================================================================================= */
/* System Set Definitions */
/*                                                                                   */
/* We use empty structs as unique types to identify our system sets.               */
/* ================================================================================= */

struct InputSet {
};
struct PhysicsSet {
};
struct RenderSet {
};

/* ================================================================================= */
/* System Definitions */
/*                                                                                   */
/* Each system simply prints a message to trace its execution.                     */
/* ================================================================================= */

/* --- Systems belonging to the InputSet --- */
void system_read_keyboard()
{
    std::cout << "[InputSet]   Reading keyboard state." << std::endl;
}
void system_read_mouse()
{
    std::cout << "[InputSet]   Reading mouse state." << std::endl;
}

/* --- Systems belonging to the PhysicsSet --- */
void system_update_positions()
{
    std::cout << "[PhysicsSet] Updating entity positions." << std::endl;
}
void system_detect_collisions()
{
    std::cout << "[PhysicsSet] Detecting collisions." << std::endl;
}

/* --- Systems belonging to the RenderSet --- */
void system_render_world()
{
    std::cout << "[RenderSet]  Rendering the world." << std::endl;
}
void system_render_ui()
{
    std::cout << "[RenderSet]  Rendering UI elements." << std::endl;
}

/* --- An independent system not belonging to any set --- */
void system_independent_task()
{
    std::cout << "[Independent] Performing an independent task." << std::endl;
}

int main()
{
    std::cout << "--- Configuration Phase ---" << std::endl;
    std::cout << "Adding systems and configuring sets in a jumbled order." << std::endl;

    r::Application{} /* Add minimal plugins so the application can run. */
        .add_plugins(r::DefaultPlugins{})

        /* 1. Add rendering systems first and assign them to the RenderSet. */
        .add_systems<system_render_world, system_render_ui>(r::Schedule::UPDATE)
        .in_set<RenderSet>()

        /* 2. Add an independent system. */
        .add_systems<system_independent_task>(r::Schedule::UPDATE)

        /* 3. Add physics systems and assign them to the PhysicsSet. */
        .add_systems<system_update_positions, system_detect_collisions>(r::Schedule::UPDATE)
        .in_set<PhysicsSet>()

        /* 4. Add input systems last and assign them to the InputSet. */
        .add_systems<system_read_keyboard, system_read_mouse>(r::Schedule::UPDATE)
        .in_set<InputSet>()

        /* 5. Configure the ordering for the sets. */
        /* This is the core of the feature. We state the dependencies between sets. */
        .configure_sets<InputSet>(r::Schedule::UPDATE)
        .before<PhysicsSet>()

        .configure_sets<PhysicsSet>(r::Schedule::UPDATE)
        .before<RenderSet>()

        /* We can also order individual systems relative to sets. */
        /* Let's make the independent task run after all input is processed. */
        .add_systems<system_independent_task>(r::Schedule::UPDATE) /* Re-adding configures the existing system */
        .after<InputSet>()

        /* Start the application. */
        .run();

    std::cout << "--- Application Finished ---" << std::endl;

    return 0;
}
