#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Plugins/AudioPlugin.hpp>
#include <R-Engine/Plugins/DefaultPlugins.hpp>
#include <R-Engine/Plugins/InputPlugin.hpp>

// clang-format off

/** entity to identify music player, could be anything else */
struct MusicEntity {
};

/**
* startup
*/

/**
 * @brief startup audio, spawn an entity with AudioPlayer and AudioSink components to showcase AudioPlugin
 */
static void startup_audio(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio)
{
    const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg"); ///<< or anything else

    /** spawn a music entity */
    commands.spawn(
        r::AudioPlayer{ ///<< create an audio player
            audio.ptr->load(audio_path) ///<< load the music file
        },
        r::AudioSink{}, ///<< create an audio sink to control playback at runtime
        MusicEntity{} ///<< tag entity as music player
    );
}

/**
 * @brief startup input to toggle play/pause and mute/unmute
 */
static void startup_input(r::ecs::ResMut<r::InputMap> input_map)
{
    auto *map = (input_map.ptr);

    map->bindAction("TogglePlay", r::KEYBOARD, KEY_SPACE);
    map->bindAction("ToggleMute", r::KEYBOARD, KEY_M);
}

/**
* update
*/

static void update_mute(
    const r::ecs::Res<r::UserInput> user_input, ///<< InputPlugin user input
    const r::ecs::Res<r::InputMap> input_map, ///<< InputPlugin input map
    r::ecs::Query<r::ecs::Mut<r::AudioSink>, r::ecs::With<MusicEntity>> query) ///<< query for music entity's audio sink
{
    const auto *map = input_map.ptr;
    const auto input = *user_input.ptr;

    /**
    * showcase of how to toggle play/pause and mute/unmute an audio sink
    * of course you have more than 2 controls in the AudioSink component,
    * see the AudioPlugin documentation for more info
    */
    for (const auto &[sink, _] : query) {
        if (map->isActionPressed("TogglePlay", input)) {
            sink.ptr->toggle();
        }
        if (map->isActionPressed("ToggleMute", input)) {
            sink.ptr->set_mute(!sink.ptr->is_muted());
        }
    }
}

int main(void)
{
    r::Application{}
        .add_plugins(r::DefaultPlugins{}) ///<< AudioPlugin is included in DefaultPlugins
        .add_systems<startup_audio, startup_input>(r::Schedule::STARTUP)
        .add_systems<update_mute>(r::Schedule::UPDATE)
        .run();
    return 0;
}
