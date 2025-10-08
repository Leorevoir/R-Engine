#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>

struct EventA {
        i32 value = 0;
};

static void event_writer(r::ecs::EventWriter<EventA> writer)
{
    r::Logger::info("sending event...");
    writer.send(EventA{42});
}

static void event_reader(r::ecs::EventReader<EventA> reader)
{
    r::Logger::debug("reading events...");

    if (reader.has_events()) {

        for (const auto &event : reader) {
            r::Logger::info("event received: " + std::to_string(event.value));
        }

    } else {
        r::Logger::debug("no events received");
    }
}

int main(void)
{
    r::Application{}
        .add_events<EventA>()
        .add_systems<event_writer>(r::Schedule::FIXED_UPDATE)
        .add_systems<event_reader>(r::Schedule::FIXED_UPDATE)
        .after<event_writer>()
        .run();
    return 0;
}
