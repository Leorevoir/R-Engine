#pragma once

#include "R-Engine/ECS/Event.hpp"

/**
* Events
*/

template<typename EventT>
void r::ecs::Events<EventT>::send(const EventT &event)
{
    if (_reading_a) {
        _b.push_back(event);
    } else {
        _a.push_back(event);
    }
}

template<typename EventT>
void r::ecs::Events<EventT>::send(EventT &&event)
{
    if (_reading_a) {
        _b.push_back(std::move(event));
    } else {
        _a.push_back(std::move(event));
    }
}

template<typename EventT>
const std::vector<EventT> &r::ecs::Events<EventT>::get_events() const
{
    return _reading_a ? _a : _b;
}

template<typename EventT>
void r::ecs::Events<EventT>::update()
{
    /* Swap which buffer is for reading and which is for writing. */
    _reading_a = !_reading_a;

    /* Clear the new write buffer. */
    if (_reading_a) { /* We are now reading from A, so B is the write buffer. */
        _b.clear();
    } else { /* We are now reading from B, so A is the write buffer. */
        _a.clear();
    }
}

template<typename EventT>
bool r::ecs::Events<EventT>::has_events() const
{
    return _reading_a ? !_a.empty() : !_b.empty();
}

/**
* EventWriter
*/

template<typename EventT>
r::ecs::EventWriter<EventT>::EventWriter(r::ecs::Events<EventT> *events_ptr) noexcept : _events(events_ptr)
{
    /* __ctor__ */
}

template<typename EventT>
void r::ecs::EventWriter<EventT>::send(const EventT &event)
{
    if (_events) {
        _events->send(event);
    }
}

template<typename EventT>
void r::ecs::EventWriter<EventT>::send(EventT &&event)
{
    if (_events) {
        _events->send(std::move(event));
    }
}

/**
* EventReader
*/

template<typename EventT>
r::ecs::EventReader<EventT>::EventReader(const r::ecs::Events<EventT> *events_ptr) noexcept : _events(events_ptr)
{
    /* __ctor__ */
}

template<typename EventT>
bool r::ecs::EventReader<EventT>::has_events() const noexcept
{
    return _events && _events->has_events();
}

/**
* EventReader Iterator
*/

template<typename EventT>
r::ecs::EventReader<EventT>::Iterator::Iterator(const std::vector<EventT> *events, u64 idx) : _events(events), _index(idx)
{
    /* __ctor__ */
}

template<typename EventT>
typename r::ecs::EventReader<EventT>::Iterator::reference r::ecs::EventReader<EventT>::Iterator::operator*() const
{
    return (*_events)[_index];
}

template<typename EventT>
typename r::ecs::EventReader<EventT>::Iterator::pointer r::ecs::EventReader<EventT>::Iterator::operator->() const
{
    return &(*_events)[_index];
}

template<typename EventT>
typename r::ecs::EventReader<EventT>::Iterator &r::ecs::EventReader<EventT>::Iterator::operator++()
{
    ++_index;
    return *this;
}

template<typename EventT>
typename r::ecs::EventReader<EventT>::Iterator r::ecs::EventReader<EventT>::Iterator::operator++(i32)
{
    Iterator tmp = *this;

    ++_index;
    return tmp;
}

template<typename EventT>
bool r::ecs::EventReader<EventT>::Iterator::operator==(const Iterator &other) const
{
    return _index == other._index;
}

template<typename EventT>
bool r::ecs::EventReader<EventT>::Iterator::operator!=(const Iterator &other) const
{
    return _index != other._index;
}

template<typename EventT>
typename r::ecs::EventReader<EventT>::Iterator r::ecs::EventReader<EventT>::begin() const
{
    if (!_events) {
        return Iterator(nullptr, 0);
    }
    return Iterator(&_events->get_events(), 0);
}

template<typename EventT>
typename r::ecs::EventReader<EventT>::Iterator r::ecs::EventReader<EventT>::end() const
{
    if (!_events) {
        return Iterator(nullptr, 0);
    }
    return Iterator(&_events->get_events(), _events->get_events().size());
}
