#include <R-Engine/Application.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/AudioPlugin.hpp>

using AudioPlayerSinkQuery = r::ecs::Query<r::ecs::Ref<r::AudioPlayer>, r::ecs::Ref<r::AudioSink>>;

/**
 * static raylib helpers
 */

static inline bool is_sound_valid(const ::Sound *sound)
{
    return sound && sound->frameCount > 0;
}

static inline bool is_sound_playing(const ::Sound &sound)
{
    return IsSoundPlaying(sound);
}

static inline void play_sound(const ::Sound &sound)
{
    PlaySound(sound);
}

static inline void stop_sound(const ::Sound &sound)
{
    if (is_sound_playing(sound)) {
        StopSound(sound);
    }
}

static inline void pause_sound(const ::Sound &sound)
{
    if (is_sound_playing(sound)) {
        PauseSound(sound);
    }
}

static inline void resume_sound(const ::Sound &sound)
{
    ResumeSound(sound);
}

static inline void set_sound_volume(const ::Sound &sound, const f32 volume)
{
    SetSoundVolume(sound, volume);
}

static inline void set_sound_pitch(const ::Sound &sound, const f32 pitch)
{
    SetSoundPitch(sound, pitch);
}

/**
 * public systems
 */

static void audio_plugin_init_audio_device()
{
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();

        if (IsAudioDeviceReady()) {
            r::Logger::info("Audio device initialized");
        } else {
            r::Logger::error("Failed to initialize audio device");
        }
    }
}

static void audio_plugin_audio_play_system(AudioPlayerSinkQuery query, r::ecs::Res<r::AudioManager> manager)
{
    for (auto [player_ref, sink_ref] : query) {
        const auto *player = player_ref.ptr;
        const auto *sink = sink_ref.ptr;
        const ::Sound *sound = manager.ptr->get(player->id);

        if (!is_sound_valid(sound)) {
            r::Logger::warn("AudioPlayer has invalid AudioHandle");
            continue;
        }

        if (sink->is_stopped()) {
            stop_sound(*sound);
            continue;
        }

        if (sink->is_muted()) {
            pause_sound(*sound);
            continue;
        }

        if (sink->is_paused()) {
            pause_sound(*sound);
            continue;
        }

        if (!is_sound_playing(*sound)) {
            play_sound(*sound);
        } else {
            resume_sound(*sound);
        }

        set_sound_volume(*sound, sink->get_volume());
        set_sound_pitch(*sound, sink->get_pitch());
    }
}

static void audio_plugin_close_audio_device()
{
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
        r::Logger::info("Audio device closed");
    }
}

/**
* public
*/

void r::AudioPlugin::build(r::Application &app)
{
    app.insert_resource(AudioManager{})
        .add_systems<audio_plugin_init_audio_device>(Schedule::PRE_STARTUP)
        .add_systems<audio_plugin_audio_play_system>(Schedule::UPDATE)
        .add_systems<audio_plugin_close_audio_device>(Schedule::SHUTDOWN);
}
