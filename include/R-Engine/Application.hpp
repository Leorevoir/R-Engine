#pragma once

#include <R-Engine/Core/BitMask.hpp>
#include <R-Engine/Core/Clock.hpp>
#include <R-Engine/ECS/Scene.hpp>

namespace r {

class Application
{
    public:
        explicit Application(const u32 scene_count = 1);
        ~Application();

        void run();

        static inline bool quit = false;

    private:
        core::Clock _clock = {};
        core::BitMask _active_scene = {};
        std::vector<std::unique_ptr<Scene>> _scenes = {};

        void _update();
};

}// namespace r
