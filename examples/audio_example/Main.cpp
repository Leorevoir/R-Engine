#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Plugins/AudioPlugin.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>

// clang-format off

static void startup_system(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio)
{
    const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg");

    commands.spawn(
        r::AudioPlayer{
            audio.ptr->load(audio_path)
        },
        r::AudioSink{}
    );
}

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{})
        .add_systems<startup_system>(r::Schedule::STARTUP)
        .run();
    return 0;
}
