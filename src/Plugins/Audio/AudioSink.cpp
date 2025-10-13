#include <R-Engine/Application.hpp>
#include <R-Engine/Plugins/AudioPlugin.hpp>

/**
* public
*/

r::AudioSink::AudioSink(const f32 volume, const f32 pitch, const bool paused, const bool mute) noexcept
    : _volume(volume), _pitch(pitch), _paused(paused), _muted(mute)
{
    /* __ctor__ */
}

/**
 * helpers
 */

void r::AudioSink::play() noexcept
{
    _paused = false;
    _stopped = false;
}

void r::AudioSink::pause() noexcept
{
    _paused = true;
}

void r::AudioSink::stop() noexcept
{
    _stopped = true;
    _paused = false;
}

void r::AudioSink::toggle() noexcept
{
    _paused ? play() : pause();
}

/**
* setters
*/

void r::AudioSink::set_volume(const f32 volume) noexcept
{
    _volume = volume;
}

void r::AudioSink::set_pitch(const f32 pitch) noexcept
{
    _pitch = pitch;
}

void r::AudioSink::set_mute(const bool mute) noexcept
{
    _muted = mute;
}

void r::AudioSink::set_paused(const bool pause) noexcept
{
    _paused = pause;
}

/**
* getters
*/

bool r::AudioSink::is_playing() const noexcept
{
    return !_paused && !_stopped;
}

bool r::AudioSink::is_paused() const noexcept
{
    return _paused;
}

bool r::AudioSink::is_stopped() const noexcept
{
    return _stopped;
}

bool r::AudioSink::is_muted() const noexcept
{
    return _muted;
}

f32 r::AudioSink::get_volume() const noexcept
{
    return _volume;
}

f32 r::AudioSink::get_pitch() const noexcept
{
    return _pitch;
}
