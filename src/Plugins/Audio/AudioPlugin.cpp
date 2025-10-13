#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/AudioPlugin.hpp>

using AudioPlayerSinkQuery = r::ecs::Query<r::ecs::Ref<r::AudioPlayer>, r::ecs::Ref<r::AudioSink>>;

static void audio_plugin_init_audio_device()
{
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        r::Logger::info("Audio device initialized");
    }
}

static void audio_plugin_audio_play_system(AudioPlayerSinkQuery query, r::ecs::Res<r::AudioManager> manager)
{
    for (auto [player_ref, sink_ref] : query) {
        const auto *player = player_ref.ptr;
        const auto *sink = sink_ref.ptr;

        const ::Sound *sound = manager.ptr->get(player->id);

        if (!sound) {
            r::Logger::warn("AudioPlayer has invalid AudioHandle");
            continue;
        }

        if (sink->paused || sink->muted) {
            r::Logger::warn("AudioSink is paused or muted, stopping sound");
            continue;
        }

        if (!IsSoundPlaying(*sound)) {
            PlaySound(*sound);
            SetSoundVolume(*sound, sink->volume);
            SetSoundPitch(*sound, sink->pitch);
        } else {
            SetSoundVolume(*sound, sink->muted ? 0.f : sink->volume);
            SetSoundPitch(*sound, sink->pitch);
        }
    }
}

static void audio_plugin_close_audio_device()
{
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
        r::Logger::info("Audio device closed");
    }
}

void r::AudioPlugin::build(r::Application &app)
{
    app.insert_resource(AudioManager{})
        .add_systems<audio_plugin_init_audio_device>(Schedule::PRE_STARTUP)
        .add_systems<audio_plugin_audio_play_system>(Schedule::UPDATE)
        .add_systems<audio_plugin_close_audio_device>(Schedule::SHUTDOWN);
}
