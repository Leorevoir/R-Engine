#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <iostream>

// clang-format off

/* ================================================================================= */
/* Example Objective */
/* */
/* Demonstrate that the scheduler can execute systems in the correct order */
/* based on dependencies (.after() / .before()), even if the systems are */
/* added to the application in a completely different order. */
/* */
/* Desired execution order: */
/* 1. system_A_input */
/* 2. system_B_physics */
/* 3. system_C_logic */
/* 4. system_D_rendering */
/* */
/* To make the test more complex, we add an "E" system that must execute */
/* BEFORE the rendering system "D" but AFTER the physics system "B". */
/* */
/* Final expected order: A -> B -> E -> C -> D */
/* Note: The order between E and C is not specified, so C -> E is also valid. */
/* The topological sort will choose a valid order. Let's assume our algorithm */
/* will produce A->B->E->C->D. */
/* ================================================================================= */

/* ================================================================================= */
/* System Definitions */
/* */
/* Each system simply prints a message to trace its execution. */
/* ================================================================================= */

/**
 * @brief (UPDATE) Handles player input.
 * @details This system simulates processing player input and should run first
 * in the game loop to capture user interactions.
 */
void system_A_input()      { std::cout << "1. [Input] Player presses a key." << std::endl; }

/**
 * @brief (UPDATE) Updates physics simulation.
 * @details This system processes physics calculations and updates entity positions
 * based on input received from the input system.
 */
void system_B_physics()    { std::cout << "2. [Physics] Player position is updated." << std::endl; }

/**
 * @brief (UPDATE) Processes game logic.
 * @details This system handles AI decisions and game state updates that depend
 * on the current physics state.
 */
void system_C_logic()      { std::cout << "3. [Logic] Enemy AI decides to react." << std::endl; }

/**
 * @brief (UPDATE) Renders the game scene.
 * @details This system draws all entities at their updated positions and should
 * run last to display the final game state.
 */
void system_D_rendering()  { std::cout << "4. [Render] Player and AI are drawn at their new positions." << std::endl; }

/**
 * @brief (UPDATE) Generates visual effects.
 * @details This system creates particle effects and other visual feedback
 * based on movement and should run after physics but before rendering.
 */
void system_E_effects()    { std::cout << "   -> [VFX] A dust effect is created due to movement." << std::endl; }


int main()
{
    std::cout << "--- Starting application configuration ---" << std::endl;
    std::cout << "Adding systems in random order to test the scheduler..." << std::endl;

    r::Application{}
        /* Add minimal plugins so the application can run for one frame. */
        .add_plugins(r::DefaultPlugins{})

        /* Add system D (Render) first. */
        .add_systems<system_D_rendering>(r::Schedule::UPDATE)

        /* Add system B (Physics) next. */
        .add_systems<system_B_physics>(r::Schedule::UPDATE)

        /* Add system A (Input) and specify it must execute BEFORE system B. */
        /* This is the first constraint. */
        .add_systems<system_A_input>(r::Schedule::UPDATE)
        .before<system_B_physics>()

        /* Add system E (Effects) and define its two constraints: */
        /* 1. It must execute AFTER physics (B). */
        /* 2. It must execute BEFORE rendering (D). */
        .add_systems<system_E_effects>(r::Schedule::UPDATE)
        .after<system_B_physics>()
        .before<system_D_rendering>()

        /* Add system C (Logic) and specify it must execute AFTER physics (B). */
        .add_systems<system_C_logic>(r::Schedule::UPDATE)
        .after<system_B_physics>()

        /* Start the application. */
        .run();

    std::cout << "--- End of application ---" << std::endl;

    return 0;
}
// clang-format on
