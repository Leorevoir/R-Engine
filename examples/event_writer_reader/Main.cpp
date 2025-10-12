#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
#include <string>

/**
 * @brief An event that is sent to signal a score increase.
 * @details It carries a `points` value, allowing different actions
 * to award different amounts of points.
 */
struct IncrementScoreEvent {
        int points = 10;
};

/**
 * @brief A global resource to store the player's score.
 */
struct Score {
        int value = 0;
};

/**
 * @brief (UPDATE) This system is an EventWriter.
 * @details It checks for user input (spacebar press) and, if detected,
 * sends an `IncrementScoreEvent` to the application's event queue.
 */
static void input_system(r::ecs::EventWriter<IncrementScoreEvent> writer)
{
    if (IsKeyPressed(KEY_SPACE)) {
        r::Logger::info("[Input System] Spacebar pressed! Sending IncrementScoreEvent.");
        writer.send(IncrementScoreEvent{});
    }
}

/**
 * @brief (UPDATE) This system is an EventReader.
 * @details It listens for `IncrementScoreEvent`s and updates the global `Score`
 * resource whenever an event is received.
 */
static void score_system(r::ecs::EventReader<IncrementScoreEvent> reader, r::ecs::ResMut<Score> score)
{
    if (!reader.has_events()) {
        return;
    }

    r::Logger::debug("[Score System] Checking for events...");
    for (const auto &event : reader) {
        score.ptr->value += event.points;
        r::Logger::info("[Score System] Event received! New score: " + std::to_string(score.ptr->value));
    }
}

/**
 * @brief (UPDATE) A second EventReader to demonstrate the broadcast nature of events.
 * @details This system also listens for `IncrementScoreEvent`s but only logs
 * that it detected them, showing that multiple systems can react independently.
 */
static void log_events_system(r::ecs::EventReader<IncrementScoreEvent> reader)
{
    if (reader.has_events()) {
        int count = 0;
        /* The iterator can be used in a range-based for loop as shown in score_system,
        or manually like a standard iterator. */
        for (auto it = reader.begin(); it != reader.end(); ++it) {
            count++;
        }
        r::Logger::debug("[Logging System] Detected " + std::to_string(count) + " IncrementScoreEvent(s) this frame.");
    }
}

/**
 * @brief (RENDER) This system displays the current score and instructions.
 * @details It reads the `Score` resource each frame and draws it to the screen.
 * It does not need to know about the events, only the final score.
 */
static void display_system(r::ecs::Res<Score> score)
{
    DrawText("Press SPACE to increment score!", 160, 250, 20, DARKGRAY);
    std::string score_text = "Score: " + std::to_string(score.ptr->value);
    DrawText(score_text.c_str(), 350, 300, 40, BLACK);
    DrawFPS(10, 10);
}

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}.set(r::WindowPlugin{r::WindowPluginConfig{
            .size = {800, 600},
            .title = "Event Writer/Reader Example",
        }}))

        /* Insert the Score resource with an initial value of 0. */
        .insert_resource(Score{0})

        /* Register the `IncrementScoreEvent` type with the application. */
        /* This creates the necessary event buffer resource. */
        .add_events<IncrementScoreEvent>()

        /* Add the input system, which will send events. */
        .add_systems<input_system>(r::Schedule::UPDATE)

        /* Add the two event-reading systems. */
        /* Crucially, specify that they must run AFTER the input_system. This
        ensures they can process events sent in the very same frame. */
        .add_systems<score_system, log_events_system>(r::Schedule::UPDATE)
        .after<input_system>()

        /* Add the display system to the rendering schedule. */
        .add_systems<display_system>(r::Schedule::RENDER_2D)

        /* Start the application loop. */
        .run();
    return 0;
}
