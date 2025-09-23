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

Test(Plugins, AddPluginByType, .init = _redirect_all_stdout)
{
    class TestPlugin : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<int>(42);
            }
    };

    r::Application app;

    app.add_plugins<TestPlugin>();
    // Test that the system runner now correctly handles reference parameters like Commands&
    app.add_systems(r::Schedule::STARTUP, [](r::ecs::Res<int> res, r::ecs::Commands& /*cmds*/) {
        cr_assert(res.ptr != nullptr, "Resource should have been inserted by the plugin.");
        cr_assert_eq(*res.ptr, 42, "Resource value is incorrect.");
    });
}

Test(Plugins, BasicPluginGroup, .init = _redirect_all_stdout)
{
    class TestPluginA : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<int>(42);
            }
    };
    class TestPluginB : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<float>(3.14f);
            }
    };
    class TestPluginGroup : public r::PluginGroup
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

    app.add_systems(r::Schedule::STARTUP, [](r::ecs::Res<int> res_int, r::ecs::Res<float> res_float) {
        cr_assert(res_int.ptr != nullptr);
        cr_assert_eq(*res_int.ptr, 42);
        cr_assert(res_float.ptr != nullptr);
        cr_assert_float_eq(*res_float.ptr, 3.14f, 1e-6);
    });
}

Test(Plugins, BevyStyleSet, .init = _redirect_all_stdout)
{
    struct MyConfig {
            int value = 10;
    };

    class ConfigurablePlugin : public r::Plugin
    {
        public:
            ConfigurablePlugin(MyConfig config = {}) : _config(config)
            {
            }
            void build(r::Application &app) override
            {
                app.insert_resource<MyConfig>(_config);
            }

        private:
            MyConfig _config;
    };

    class UntouchedPlugin : public r::Plugin
    {
        public:
            void build(r::Application &app) override
            {
                app.insert_resource<float>(99.0f);
            }
    };

    class TestGroupWithDefaults : public r::PluginGroup
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

    app.add_systems(r::Schedule::STARTUP, [](r::ecs::Res<MyConfig> config, r::ecs::Res<float> untouched) {
        cr_assert(config.ptr != nullptr);
        cr_assert_eq(config.ptr->value, 500, "The configured value should be 500, not the default.");
        cr_assert(untouched.ptr != nullptr);
        cr_assert_float_eq(*untouched.ptr, 99.0f, 1e-6);
    });
}
