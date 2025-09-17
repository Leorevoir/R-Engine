#include "R-Engine/Plugins/DefaultPlugins.hpp"
#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Command.hpp>

// clang-format off

int main()
{
    r::Application{}.add_plugins<r::DefaultPlugins>().run();

    return 0;
}
