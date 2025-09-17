#include "../Test.hpp"

#include <R-Engine/Application.hpp>

#include <criterion/redirect.h>

static void _redirect_all_stdout()
{
    cr_redirect_stdout();
    cr_redirect_stderr();
}

Test(Plugins, Plugin, .init = _redirect_all_stdout)
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
    app.add_systems(r::Schedule::STARTUP, [](r::ecs::Res<int> res) {
        cr_assert(res.ptr != nullptr);
        cr_assert(*res.ptr == 42);
    });
}

Test(Plugins, PluginGroup, .init = _redirect_all_stdout)
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
            void build(r::Application &app) override
            {
                app.add_plugins<TestPluginA>();
                app.add_plugins<TestPluginB>();
            }
    };
    r::Application app;
    app.add_plugins<TestPluginGroup>();
    app.add_systems(r::Schedule::STARTUP, [](r::ecs::Res<int> res_int, r::ecs::Res<float> res_float) {
        cr_assert(res_int.ptr != nullptr);
        cr_assert(*res_int.ptr == 42);
        cr_assert(res_float.ptr != nullptr);
        cr_assert(*res_float.ptr == 3.14f);
    });
}
