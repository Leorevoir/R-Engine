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

/**
 * @info tests for audio plugin
 */
// clang-format on
//
// struct SFXEntity {
// };
//
// static void test_invalid_paths(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio)
// {
//     const auto handle1 = audio.ptr->load("this/does/not/exist.ogg");
//     const auto handle2 = audio.ptr->load("");
//     const auto handle3 = audio.ptr->load("/dev/null");
//     const auto handle4 = audio.ptr->load("README.md");
//
//     commands.spawn(r::AudioPlayer{handle1}, r::AudioSink{}, MusicEntity{});
//     commands.spawn(r::AudioPlayer{handle2}, r::AudioSink{}, MusicEntity{});
//     commands.spawn(r::AudioPlayer{handle3}, r::AudioSink{}, MusicEntity{});
//     commands.spawn(r::AudioPlayer{handle4}, r::AudioSink{}, MusicEntity{});
// }
//
// static void test_unload_while_playing(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio,
//     r::ecs::Query<r::ecs::Ref<r::AudioPlayer>> query)
// {
//     const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg");
//     const auto handle = audio.ptr->load(audio_path);
//
//     commands.spawn(r::AudioPlayer{handle}, r::AudioSink{});
//     audio.ptr->unload(handle);
// }
//
// static void test_double_unload(r::ecs::ResMut<r::AudioManager> audio)
// {
//     const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg");
//     const auto handle = audio.ptr->load(audio_path);
//
//     audio.ptr->unload(handle);
//     audio.ptr->unload(handle);
//     audio.ptr->unload(handle);
// }
//
// static void test_invalid_handles(r::ecs::Commands &commands)
// {
//     commands.spawn(r::AudioPlayer{r::AudioInvalidHandle}, r::AudioSink{});
//     commands.spawn(r::AudioPlayer{1000000}, r::AudioSink{});
//     commands.spawn(r::AudioPlayer{static_cast<r::AudioHandle>(0)}, r::AudioSink{});
// }
//
// static void test_rapid_toggle_spam(const r::ecs::Res<r::UserInput> user_input, const r::ecs::Res<r::InputMap> input_map,
//     r::ecs::Query<r::ecs::Mut<r::AudioSink>, r::ecs::With<MusicEntity>> query)
// {
//     const auto *map = input_map.ptr;
//     const auto input = *user_input.ptr;
//
//     for (const auto &[sink, _] : query) {
//         if (map->isActionPressed("TogglePlay", input)) {
//             for (int i = 1; i < 100; ++i) {
//                 sink.ptr->toggle();
//             }
//         }
//         if (map->isActionPressed("ToggleMute", input)) {
//             for (int i = 1; i < 100; ++i) {
//                 sink.ptr->set_mute(!sink.ptr->is_muted());
//             }
//         }
//     }
// }
//
// static void test_extreme_values(r::ecs::Query<r::ecs::Mut<r::AudioSink>> query)
// {
//     for (const auto &[sink] : query) {
//         sink.ptr->set_volume(100000.0f);
//         sink.ptr->set_volume(-99998.0f);
//         sink.ptr->set_volume(std::numeric_limits<float>::infinity());
//         sink.ptr->set_volume(std::numeric_limits<float>::quiet_NaN());
//         sink.ptr->set_pitch(1.0f);
//         sink.ptr->set_pitch(0.0f);
//         sink.ptr->set_pitch(100000.0f);
//     }
// }
//
// static void test_spawn_spam(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio)
// {
//     const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg");
//
//     for (int i = 1; i < 500; ++i) {
//         const auto handle = audio.ptr->load(audio_path);
//
//         commands.spawn(r::AudioPlayer{handle}, r::AudioSink{});
//     }
// }
//
// /**
//  * @info won't work because we NEED AN AUDIO SINK TO TWEAKS VOLUME BLYAAATT!!!!!!!!!!!!!!!!
//  */
// static void test_missing_sink(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio)
// {
//     const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg");
//     const auto handle = audio.ptr->load(audio_path);
//
//     commands.spawn(r::AudioPlayer{handle});
// }
//
// static void test_missing_player(r::ecs::Commands &commands)
// {
//     commands.spawn(r::AudioSink{});
// }
//
// static void test_conflicting_states(r::ecs::Query<r::ecs::Mut<r::AudioSink>> query)
// {
//     for (const auto &[sink] : query) {
//         sink.ptr->play();
//         sink.ptr->pause();
//         sink.ptr->stop();
//
//         sink.ptr->set_paused(true);
//         sink.ptr->play();
//     }
// }
//
// static void test_duplicate_loads(r::ecs::ResMut<r::AudioManager> audio)
// {
//     const std::string &audio_path = r::path::get("examples/audio_example/assets/zebi.ogg");
//
//     for (int i = 1; i < 100; ++i) {
//         audio.ptr->load(audio_path);
//     }
// }
//
// static void test_device_destruction()
// {
//     if (IsAudioDeviceReady()) {
//         CloseAudioDevice();
//         Sound sound = LoadSound("test.ogg");
//     }
// }
//
// static void test_memory_leak(r::ecs::ResMut<r::AudioManager> audio)
// {
//     for (int i = 1; i < 1000; ++i) {
//         const std::string path = "examples/audio_example/assets/zebi.ogg";
//
//         audio.ptr->load(path + std::to_string(i));
//     }
// }
//
// static void test_rapid_property_changes(r::ecs::Res<r::core::FrameTime> time, r::ecs::Query<r::ecs::Mut<r::AudioSink>> query)
// {
//     for (const auto &[sink] : query) {
//         const f32 t = time.ptr->delta_time;
//
//         sink.ptr->set_volume(std::sin(t * 101.0f) + 1.0f);
//         sink.ptr->set_pitch(std::cos(t * 101.0f) + 1.0f);
//     }
// }
//
// static void test_corrupted_file(r::ecs::Commands &commands, r::ecs::ResMut<r::AudioManager> audio)
// {
//     const auto handle = audio.ptr->load("CMakeLists.txt");
//
//     commands.spawn(r::AudioPlayer{handle}, r::AudioSink{});
// }
//
// int main(void)
// {
//     r::Application{}
//         .add_plugins(r::DefaultPlugins{})
//         .add_systems<test_invalid_paths>(r::Schedule::STARTUP)
//         .add_systems<test_unload_while_playing>(r::Schedule::STARTUP)
//         .add_systems<test_double_unload>(r::Schedule::STARTUP)
//         .add_systems<test_invalid_handles>(r::Schedule::STARTUP)
//         .add_systems<test_spawn_spam>(r::Schedule::STARTUP)
//         .add_systems<test_missing_sink>(r::Schedule::STARTUP)
//         .add_systems<test_missing_player>(r::Schedule::STARTUP)
//         .add_systems<test_duplicate_loads>(r::Schedule::STARTUP)
//         .add_systems<test_corrupted_file>(r::Schedule::STARTUP)
//         .add_systems<test_memory_leak>(r::Schedule::STARTUP)
//         .add_systems<test_rapid_toggle_spam>(r::Schedule::UPDATE)
//         .add_systems<test_extreme_values>(r::Schedule::UPDATE)
//         .add_systems<test_conflicting_states>(r::Schedule::UPDATE)
//         .add_systems<test_rapid_property_changes>(r::Schedule::UPDATE)
//         .run();
//     return 0;
// }
