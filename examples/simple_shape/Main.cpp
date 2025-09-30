#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>

/* A simple system to draw a static shape. */
void render_system()
{
    DrawRectangle(100, 100, 200, 100, DARKBLUE);
    DrawText("This is the 'simple_shape' example", 10, 10, 20, BLACK);
}

int main()
{
    r::Application{}.add_plugins(r::DefaultPlugins{}).add_systems(r::Schedule::RENDER, render_system).run();
    return 0;
}
