#include <R-Engine/Application.hpp>
#include <R-Engine/ECS/Event.hpp>

#include <iostream>

struct EventA {
        i32 value = 0;
};

static void event_writer(r::ecs::EventWriter<EventA> writer)
{
    writer.send(EventA{42});
}

static void event_reader(r::ecs::EventReader<EventA> reader)
{
    for (const auto &event : reader) {
        std::cout << "received: " << event.value << std::endl;
    }
}

int main(void)
{
    r::Application{}.add_events<EventA>().add_systems<event_writer, event_reader>(r::Schedule::FIXED_UPDATE).run();
    return 0;
}
