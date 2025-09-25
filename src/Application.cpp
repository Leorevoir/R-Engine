#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <csignal>
#include <iostream>

/**
* public
*/

r::Application::Application()
{
    Logger::info("Application created");
    std::signal(SIGINT, [](int) {
        std::cout << "\r";
        Logger::info("SIGINT received, quitting application...");
        r::Application::quit = true;
    });
}

void r::Application::Application::run()
{
    _scene.insert_resource<core::FrameTime>(_clock.frame());

    Logger::debug("Startup schedule running...");
    _run_schedule(Schedule::STARTUP);
    _apply_commands();
    Logger::debug("Startup schedule complete. Entering main loop.");

    while (!quit) {
        _clock.tick();
        *_scene.get_resource_ptr<core::FrameTime>() = _clock.frame();

        _run_schedule(Schedule::UPDATE);
        _apply_commands();

        for (i32 i = 0; i < _clock.frame().substep_count; ++i) {
            _run_schedule(Schedule::FIXED_UPDATE);
            _apply_commands();
        }
        _run_schedule(Schedule::RENDER);
    }

    Logger::debug("Main loop exited. Running shutdown schedule...");
    _run_schedule(Schedule::SHUTDOWN);
    _apply_commands();
    Logger::debug("Shutdown schedule complete. Application exiting.");
}

/**
* private
*/

void r::Application::_run_schedule(const Schedule sched)
{
    const auto it = _systems.find(sched);

    if (it == _systems.end()) {
        return;
    }
    for (const auto &sys : it->second) {
        sys(_scene, _command_buffer);
    }
}

void r::Application::_apply_commands()
{
    _command_buffer.apply(_scene);
}
