#pragma once

namespace r {

namespace ecs {

template<typename EventT>
class EventWriter;

template<typename EventT>
class EventReader;

template<typename EventT>
class EventWriter final
{
    public:
        EventWriter() noexcept = default;

        void send(const EventT &event);

    private:
};

template<typename EventT>
class EventReader final
{
    public:
        EventReader() noexcept = default;

        bool has_events() const noexcept;
        EventT read();

    private:
};

}// namespace ecs

}// namespace r
