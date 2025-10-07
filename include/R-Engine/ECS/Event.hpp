#pragma once

#include <R-Engine/Types.hpp>

#include <vector>

namespace r {

namespace ecs {

template<typename EventT>
class EventWriter;

template<typename EventT>
class EventReader;

/**
 * @brief Internal storage for events of type EventT.
 * @details This is stored as a resource in the Scene.
 * Uses double buffering to avoid iterator invalidation during reads.
 */
template<typename EventT>
class Events
{
    public:
        Events() = default;

        void send(const EventT &event);
        void send(EventT &&event);

        const std::vector<EventT> &get_events() const;
        void clear();
        bool has_events() const;

    private:
        std::vector<EventT> _events;
};

/**
 * @brief Provides write access to send events of type EventT.
 * @details EventWriter is resolved by the Resolver as a wrapper around Events<EventT> resource.
 */
template<typename EventT>
class EventWriter final
{
    public:
        EventWriter() noexcept = default;
        explicit EventWriter(Events<EventT> *events_ptr) noexcept;

        void send(const EventT &event);
        void send(EventT &&event);

    private:
        Events<EventT> *_events = nullptr;
};

/**
 * @brief Provides read access to iterate over events of type EventT.
 * @details EventReader is resolved by the Resolver as a wrapper around Events<EventT> resource.
 */
template<typename EventT>
class EventReader final
{
    public:
        EventReader() noexcept = default;
        explicit EventReader(const Events<EventT> *events_ptr) noexcept;

        bool has_events() const noexcept;

        class Iterator
        {
            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type = EventT;
                using difference_type = std::ptrdiff_t;
                using pointer = const EventT *;
                using reference = const EventT &;

                Iterator(const std::vector<EventT> *events, u64 idx);
                reference operator*() const;
                pointer operator->() const;

                Iterator &operator++();
                Iterator operator++(i32);

                bool operator==(const Iterator &other) const;
                bool operator!=(const Iterator &other) const;

            private:
                const std::vector<EventT> *_events;
                u64 _index;
        };

        Iterator begin() const;
        Iterator end() const;

    private:
        const Events<EventT> *_events = nullptr;
};

}// namespace ecs

}// namespace r

#include "Inline/Event.inl"
