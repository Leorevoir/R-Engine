#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Clock.hpp>

/**
* public
*/

void r::Application::Application::run()
{
    _scene.insert_resource<core::FrameTime>(_clock.frame());
    _run_schedule(Schedule::STARTUP);

    while (!quit) {
        _clock.tick();
        *_scene.get_resource_ptr<core::FrameTime>() = _clock.frame();

        _run_schedule(Schedule::UPDATE);

        for (i32 i = 0; i < _clock.frame().substep_count; ++i) {
            _run_schedule(Schedule::FIXED_UPDATE);
        }
    }
}

/**
* private
*/
