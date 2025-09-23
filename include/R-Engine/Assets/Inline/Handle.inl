#pragma once

template<typename T>
constexpr Handle<T>::Handle(const IdType id) noexcept : _id(id)
{
    /* __ctor__ */
}

template<typename T>
bool Handle<T>::valid() const noexcept
{
    return _id != InvalidId;
}

template<typename T>
u32 Handle<T>::id() const noexcept
{
    return _id;
}

template<typename T>
bool Handle<T>::operator==(const Handle<T> &other) const noexcept
{
    return _id == other._id;
}

template<typename T>
bool Handle<T>::operator!=(const Handle<T> &other) const noexcept
{
    return _id != other._id;
}
