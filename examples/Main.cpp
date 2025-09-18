#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>
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
        .add_plugins(
            r::DefaultPlugins{}
                .set(r::WindowPlugin{
                    r::WindowPluginConfig{
                        .size = {800, 600},
                        .title = "Bevy Style Configuration!",
                    }
                })
        )
        /* Adding the whole PluginGroup with defaults
        .add_plugins<r::DefaultPlugins>() */

        /* Adding the plugins separately with defaults
        .add_plugins<r::WindowPlugin, r::RenderPlugin>() */

        .add_systems(r::Schedule::FIXED_UPDATE, test_system, other_test_system)
        .run();

    return 0;
}
