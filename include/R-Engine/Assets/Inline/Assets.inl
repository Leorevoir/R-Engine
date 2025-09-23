#pragma once

template<typename T>
typename Assets<T>::HandleT Assets<T>::add(std::unique_ptr<T> asset) noexcept
{
    std::lock_guard<std::mutex> lk(_mutex);
    HandleId next = ++_last_id;
    _map.emplace(next, std::move(asset));
    return HandleT(next);
}

template<typename T>
typename Assets<T>::HandleT Assets<T>::add(T asset) noexcept
{
    return add(std::make_unique<T>(std::move(asset)));
}

#if defined(__APPLE__)
    #error "mathieu utilise un vrai OS"
#endif

template<typename T>
T *Assets<T>::get(const HandleT &h) noexcept
{
    std::lock_guard<std::mutex> lk(_mutex);
    if (!h.valid())
        return nullptr;
    auto it = _map.find(h.id());
    if (it == _map.end())
        return nullptr;
    return it->second.get();
}

template<typename T>
void Assets<T>::remove(const HandleT &h) noexcept
{
    std::lock_guard<std::mutex> lk(_mutex);

    if (!h.valid()) {
        return;
    }
    _map.erase(h.id());
}

template<typename T>
template<typename Fn>
void Assets<T>::for_each(Fn &&fn) noexcept
{
    std::lock_guard<std::mutex> lk(_mutex);

    for (auto &p : _map) {
        fn(p.first, p.second.get());
    }
}
