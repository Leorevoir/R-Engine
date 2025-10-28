#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/Core/Error.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Core/ThreadPool.hpp>

#include <csignal>
#include <iostream>
#include <thread>

/**
* public
*/

r::Application::Application()
{
    Logger::debug("Application created");
    const size_t thread_count = std::thread::hardware_concurrency();

    _thread_pool = std::make_unique<core::ThreadPool>(thread_count > 0 ? thread_count : 1);
    _scheduler = std::make_unique<core::Scheduler>(*_thread_pool);

    _prepare_thread_local_buffers(thread_count > 0 ? thread_count : 1);

    #if !defined(ECS_SERVER_MODE)
        std::signal(SIGINT, [](i32) {
            r::Application::quit.store(true, std::memory_order_relaxed);
            std::cout << "\r";
            Logger::warn("SIGINT received, quitting application...");
        });
    #else
        Logger::debug("Server mode build detected, SIGINT handler is disabled for the ECS.");
    #endif
}

void r::Application::run()
{
    _startup();
    _main_loop();
    _shutdown();
}

/**
* private
*/

void r::Application::_startup()
{
    _scene.insert_resource(_clock.frame());

    Logger::debug("Pre-startup schedule running...");
    _run_schedule(Schedule::PRE_STARTUP);
    _apply_commands();

    _systems.erase(Schedule::PRE_STARTUP);
    if (_systems.empty()) {
        quit.store(true, std::memory_order_relaxed);
        return;
    }

    Logger::debug("Startup schedule running...");
    _run_schedule(Schedule::STARTUP);
    _apply_commands();
    Logger::debug("Startup schedule complete. Entering main loop.");

    _systems.erase(Schedule::STARTUP);
    if (_systems.empty()) {
        quit.store(true, std::memory_order_relaxed);
        return;
    }
}

void r::Application::_main_loop()
{
    while (!quit) {
        _clock.tick();
        *_scene.get_resource_ptr<core::FrameTime>() = _clock.frame();

        _apply_state_transitions();

        _run_schedule(Schedule::UPDATE);
        _apply_commands();

        for (i32 i = 0; i < _clock.frame().substep_count; ++i) {
            _run_schedule(Schedule::FIXED_UPDATE);
            _apply_commands();
        }
        _render_routine();
        _run_schedule(Schedule::EVENT_CLEANUP);
    }
}

void r::Application::_shutdown()
{
    Logger::debug("Main loop exited. Running shutdown schedule...");
    _run_schedule(Schedule::SHUTDOWN);
    _apply_commands();
    Logger::debug("Shutdown schedule complete. Application exiting.");
    Logger::debug("Quiting the application...");
}

/**
* private
*/

void r::Application::_run_schedule(const Schedule sched)
{
    const auto it = _systems.find(sched);

    if (it == _systems.end() || it->second.nodes.empty()) {
        return;
    }

    auto &graph = it->second;

    _scheduler->run(graph, _scene, _command_buffer, _thread_local_command_buffers);
}

void r::Application::_render_routine()
{
    _run_schedule(Schedule::BEFORE_RENDER_2D);
    _run_schedule(Schedule::BEFORE_RENDER_3D);

    _run_schedule(Schedule::RENDER_3D);

    _run_schedule(Schedule::AFTER_RENDER_3D);
    _run_schedule(Schedule::RENDER_2D);

    _run_schedule(Schedule::AFTER_RENDER_2D);
}

void r::Application::_apply_commands()
{
    _command_buffer.apply(_scene);
    for (auto &buffer : _thread_local_command_buffers) {
        buffer->apply(_scene);
    }
}

void r::Application::_apply_state_transitions()
{
    if (_state_transition_runner) {
        _state_transition_runner();
    }
}

void r::Application::_run_transition_schedule(sys::ScheduleGraph &graph)
{
    if (graph.nodes.empty()) {
        return;
    }

    _scheduler->run(graph, _scene, _command_buffer, _thread_local_command_buffers);
}

void r::Application::_prepare_thread_local_buffers(size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        _thread_local_command_buffers.push_back(std::make_unique<ecs::CommandBuffer>());
    }
}
