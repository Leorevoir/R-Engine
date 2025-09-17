#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <iostream>

static void other_test_system()
{
    std::cout << "Other hello, World!" << std::endl;
}

static void test_system()
{
    std::cout << "Hello, World!" << std::endl;
}

// clang-format off

int main()
{
    r::Application{}
        /* Adding the whole PluginGroup */
        .add_plugins<r::DefaultPlugins>()
        /* Adding the plugins separately */
        /* .add_plugins<r::WindowPlugin, r::RenderPlugin>() */
        .add_systems(r::Schedule::FIXED_UPDATE, test_system, other_test_system)
        .run();

    return 0;
}
