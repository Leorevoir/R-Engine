#pragma once

template<class F, class... Args>
auto r::core::ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        if (_stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        _tasks.emplace([task]() { (*task)(); });
    }
    _condition.notify_one();
    return res;
}
