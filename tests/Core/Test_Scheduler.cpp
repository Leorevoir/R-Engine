#include "../Test.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/Core/States.hpp>
#include <R-Engine/ECS/Event.hpp>
#include <R-Engine/ECS/RunConditions.hpp>
#include <criterion/redirect.h>

// --- Setup ---

static void _redirect_all_stdout()
{
    cr_redirect_stdout();
    cr_redirect_stderr();
}

// State enum for testing `in_state` and `state_changed`.
enum class TestState { A, B };

// A resource for testing `resource_exists`.
struct TestResource {
};

// An event for testing `on_event`.
struct TestEvent {
};

// A tracker to observe the side effects of systems.
struct TestTracker {
        int run_if_in_state_a_ran = 0;
        int run_if_state_changed_ran = 0;
        int run_if_resource_exists_ran = 0;
        int run_if_on_event_ran = 0;
        int frame_counter = 0;
};

// --- Test Systems ---

void sys_in_state_a(r::ecs::ResMut<TestTracker> tracker)
{
    tracker.ptr->run_if_in_state_a_ran++;
}
void sys_state_changed(r::ecs::ResMut<TestTracker> tracker)
{
    tracker.ptr->run_if_state_changed_ran++;
}
void sys_resource_exists(r::ecs::ResMut<TestTracker> tracker)
{
    tracker.ptr->run_if_resource_exists_ran++;
}
void sys_on_event(r::ecs::ResMut<TestTracker> tracker)
{
    tracker.ptr->run_if_on_event_ran++;
}

// This system drives the test logic frame by frame.
void driver_system(r::ecs::ResMut<TestTracker> tracker, r::ecs::ResMut<r::NextState<TestState>> next_state, r::ecs::Commands commands,
    r::ecs::EventWriter<TestEvent> writer)
{
    tracker.ptr->frame_counter++;
    const int frame = tracker.ptr->frame_counter;

    switch (frame) {
        case 1:
            // Initial state. `in_state<A>` should run.
            break;
        case 2:
            // Add the resource. `resource_exists` should now run.
            commands.insert_resource(TestResource{});
            break;
        case 3:
            // Send an event. `on_event` should run this frame.
            writer.send(TestEvent{});
            break;
        case 4:
            // Trigger a state change. `state_changed` should run next frame.
            next_state.ptr->set(TestState::B);
            break;
        case 5:
            // `in_state<A>` should stop. `state_changed` should run.
            // Remove the resource. `resource_exists` should stop next frame.
            commands.remove_resource<TestResource>();
            break;
        case 6:
            // Final frame to check that disabled systems did not run.
            r::Application::quit = true;// End the test.
            break;
    }
}

// --- Test Suite ---

Test(Scheduler, RunIfConditions, .init = _redirect_all_stdout)
{
    // Ensure the quit flag is reset before the test.
    r::Application::quit = false;

    r::Application app;
    app.insert_resource(TestTracker{})
        .init_state(TestState::A)
        .add_events<TestEvent>()

        // Add the systems with their respective run conditions.
        .add_systems<sys_in_state_a>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::in_state<TestState::A>>()

        .add_systems<sys_state_changed>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::state_changed<TestState>>()

        .add_systems<sys_resource_exists>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::resource_exists<TestResource>>()

        .add_systems<sys_on_event>(r::Schedule::UPDATE)
        .run_if<r::run_conditions::on_event<TestEvent>>()

        // Add the driver system that controls the test flow.
        .add_systems<driver_system>(r::Schedule::UPDATE)
        .run();

    // After the app loop finishes, get the final state of the tracker.
    const auto &tracker = *app.get_resource_ptr<TestTracker>();

    // Assert that each system ran the correct number of times.
    cr_assert_eq(tracker.run_if_in_state_a_ran, 4, "in_state<A> should run for frames 1, 2, 3, 4 before the state changes.");

    cr_assert_eq(tracker.run_if_state_changed_ran, 1, "state_changed should run exactly once, on the frame of the transition (frame 5).");

    cr_assert_eq(tracker.run_if_resource_exists_ran, 3,
        "resource_exists should run on frames 2, 3, 4, 5 after being added and before being removed.");

    cr_assert_eq(tracker.run_if_on_event_ran, 1, "on_event should run exactly once, on the frame an event was sent (frame 3).");
}
