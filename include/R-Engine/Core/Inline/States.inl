#pragma once

#include <R-Engine/Core/States.hpp>

template<typename T>
T r::State<T>::current() const
{
    return _current;
}

template<typename T>
std::optional<T> r::State<T>::previous() const
{
    return _previous;
}

template<typename T>
void r::NextState<T>::set(T next_state)
{
    next = next_state;
}