#include <R-Engine/Application.hpp>
#include <csignal>

/**
* public
*/

r::Application::Application(const u32 scene_count)
{
    _scenes.reserve(scene_count);

    std::signal(SIGINT, [](i32) -> void { Application::quit = true; });
}

r::Application::~Application()
{
    _scenes.clear();
}

void r::Application::run()
{
    while (!quit) {
        _clock.tick();
        _update();
    }
}

/**
* private
*/

void r::Application::_update()
{
    for (u64 i = 0; i < _scenes.size(); ++i) {
        if (!_active_scene[i]) {
            continue;
        }
        if (!_scenes[i]->update(_clock.frame())) {
            _active_scene.reset(i);
        }
    }

    quit = _active_scene.empty();
}
