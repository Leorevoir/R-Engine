#include "../Test.hpp"

#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/Plugin.hpp>

#include <criterion/redirect.h>

static void _redirect_all_stdout()
{
    cr_redirect_stdout();
    cr_redirect_stderr();
}

static void assert_plugin_resource_system(r::ecs::Res<int> res)
{
    cr_assert(res.ptr != nullptr, "Resource should have been inserted by the plugin.");
    cr_assert_eq(*res.ptr, 42, "Resource value is incorrect.");
}

Test(Plugins, AddPluginByType, .init = _redirect_all_stdout)
{
    class TestPlugin final : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<int>(42);
            }
    };

    r::Application app;

    app.add_plugins(TestPlugin{});
    app.add_systems<assert_plugin_resource_system>(r::Schedule::STARTUP);
}

static void assert_plugin_group_resources_system(r::ecs::Res<int> res_int, r::ecs::Res<float> res_float)
{
    cr_assert(res_int.ptr != nullptr);
    cr_assert_eq(*res_int.ptr, 42);
    cr_assert(res_float.ptr != nullptr);
    cr_assert_float_eq(*res_float.ptr, 3.14f, 1e-6);
}

Test(Plugins, BasicPluginGroup, .init = _redirect_all_stdout)
{
    class TestPluginA final : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<int>(42);
            }
    };
    class TestPluginB final : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<float>(3.14f);
            }
    };
    class TestPluginGroup final : public r::PluginGroup
    {
        public:
            TestPluginGroup()
            {
                add<TestPluginA>();
                add<TestPluginB>();
            }
    };

    r::Application app;
    app.add_plugins(TestPluginGroup{});

    app.add_systems<assert_plugin_group_resources_system>(r::Schedule::STARTUP);
}

struct MyConfig {
        int value = 10;
};

static void assert_bevy_style_set_system(r::ecs::Res<MyConfig> config, r::ecs::Res<float> untouched)
{
    cr_assert(config.ptr != nullptr);
    cr_assert_eq(config.ptr->value, 500, "The configured value should be 500, not the default.");
    cr_assert(untouched.ptr != nullptr);
    cr_assert_float_eq(*untouched.ptr, 99.0f, 1e-6);
}

Test(Plugins, BevyStyleSet, .init = _redirect_all_stdout)
{
    class ConfigurablePlugin final : public r::Plugin
    {
        public:
            ConfigurablePlugin(MyConfig config = {}) : _config(config)
            {
            }
            void build(r::Application &app) override
            {
                app.insert_resource(_config);
            }

        private:
            MyConfig _config;
    };

    class UntouchedPlugin final : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<float>(99.0f);
            }
    };

    class TestGroupWithDefaults final : public r::PluginGroup
    {
        public:
            TestGroupWithDefaults()
            {
                add<ConfigurablePlugin>();
                add<UntouchedPlugin>();
            }
    };

    r::Application app;

    app.add_plugins(TestGroupWithDefaults{}.set(ConfigurablePlugin{MyConfig{.value = 500}}));

    app.add_systems<assert_bevy_style_set_system>(r::Schedule::STARTUP);
}

enum class TestState { A, B };

struct TestStateTracker {
        bool on_enter_b_called = false;
        bool on_exit_a_called = false;
        bool on_transition_a_to_b_called = false;
        int frame = 0;
};

static void on_exit_a_system(r::ecs::ResMut<TestStateTracker> tracker)
{

    tracker.ptr->on_exit_a_called = true;
}

static void on_enter_b_system(r::ecs::ResMut<TestStateTracker> tracker)
{

    tracker.ptr->on_enter_b_called = true;
}

static void on_transition_a_to_b_system(r::ecs::ResMut<TestStateTracker> tracker)
{

    tracker.ptr->on_transition_a_to_b_called = true;
}

static void test_driver_system(r::ecs::Res<r::State<TestState>> state, r::ecs::ResMut<r::NextState<TestState>> next_state,
    r::ecs::ResMut<TestStateTracker> tracker)
{
    tracker.ptr->frame++;

    if (tracker.ptr->frame == 1) {

        cr_assert_eq(state.ptr->current(), TestState::A, "Should start in state A.");
        next_state.ptr->set(TestState::B);
    } else if (tracker.ptr->frame == 2) {

        cr_assert_eq(state.ptr->current(), TestState::B, "Should have transitioned to state B.");

        cr_assert(tracker.ptr->on_exit_a_called, "OnExit(A) system did not run.");
        cr_assert(tracker.ptr->on_enter_b_called, "OnEnter(B) system did not run.");
        cr_assert(tracker.ptr->on_transition_a_to_b_called, "OnTransition(A, B) system did not run.");

        r::Application::quit = true;
    }
}

Test(Plugins, StateSchedules, .init = _redirect_all_stdout)
{

    r::Application::quit = false;

    r::Application{}
        .insert_resource(TestStateTracker{})
        .init_state(TestState::A)

        .add_systems<on_exit_a_system>(r::OnExit(TestState::A))
        .add_systems<on_enter_b_system>(r::OnEnter(TestState::B))
        .add_systems<on_transition_a_to_b_system>(r::OnTransition(TestState::A, TestState::B))

        .add_systems<test_driver_system>(r::Schedule::UPDATE)

        .run();

    cr_assert(r::Application::quit, "Application did not quit as expected.");
}
