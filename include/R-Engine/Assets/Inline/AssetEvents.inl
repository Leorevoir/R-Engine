#pragma once

template<typename T>
void AssetEvents<T>::push(typename AssetEvent<T>::Type type, Handle<T> h) noexcept
{
    events.push_back({type, h});
}

template<typename T>
const std::vector<AssetEvent<T>> &AssetEvents<T>::read() const noexcept
{
    return events;
}

template<typename T>
void AssetEvents<T>::clear() noexcept
{
    events.clear();
}
