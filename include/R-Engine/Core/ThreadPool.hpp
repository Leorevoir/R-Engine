#pragma once

#include <R-Engine/R-EngineExport.hpp>

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace r {

namespace core {

/**
* @brief Thread pool for managing and executing tasks concurrently.
*/
class R_ENGINE_API ThreadPool
{
    public:
        explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
        ~ThreadPool();

        template<class F, class... Args>
        auto enqueue(F &&f, Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    private:
        std::vector<std::thread> _workers;
        std::queue<std::function<void()>> _tasks;
        std::mutex _queue_mutex;
        std::condition_variable _condition;
        bool _stop;

        void _arbeit();
};

}// namespace core

}// namespace r

#include "Inline/ThreadPool.inl"
