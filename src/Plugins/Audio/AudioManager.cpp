#include <R-Engine/Plugins/AudioPlugin.hpp>

#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>

/**
* public
*/

r::AudioManager::~AudioManager()
{
    for (auto &s : _sounds) {

        if (s.frameCount > 0) {
            UnloadSound(s);
        }
    }

    _sounds.clear();
    _audios.clear();
}

// static std::ostream &operator<<(std::ostream &os, const ::Sound *sound)
// {
//     const ::AudioStream &stream = sound->stream;
//
//     os << "@Sound{" << std::endl
//        << "\tframeCount{" << sound->frameCount << "}," << std::endl
//        << "\tstream{"
//        << "\n\t\tsampleRate{" << stream.sampleRate << "},"
//        << "\n\t\tsampleSize{" << stream.sampleSize << "},"
//        << "\n\t\tchannels{" << stream.channels << "},"
//        << "\n\t\tbuffer{" << stream.buffer << "},"
//        << "\n\t}"
//        << "\n}";
//     return os;
// }

r::AudioHandle r::AudioManager::load(const std::string &path)
{
    if (!path::exists(path)) {
        Logger::error("Audio file does not exist: " + path);
        return AudioInvalidHandle;
    }

    if (const auto it = _audios.find(path); it != _audios.end()) {
        return it->second;
    }

    const auto &sound = LoadSound(path.c_str());
    _sounds.push_back(sound);

    if (sound.frameCount == 0) {
        Logger::error("Failed to load sound: " + path);
        _sounds.pop_back();
        return AudioInvalidHandle;
    }

    const AudioHandle handle = static_cast<AudioHandle>(_sounds.size() - 1);
    _audios[path] = handle;

    Logger::debug("Loaded sound: " + path + " (" + std::to_string(handle) + ")");
    return handle;
}

void r::AudioManager::unload(const AudioHandle handle) noexcept
{
    if (handle >= _sounds.size()) {
        return;
    }

    auto &sound = _sounds[handle];

    if (sound.frameCount == 0) {
        return;
    }

    UnloadSound(sound);
    sound.frameCount = 0;
}

const ::Sound *r::AudioManager::get(AudioHandle handle) const noexcept
{
    if (handle >= _sounds.size()) {
        return nullptr;
    }
    return &_sounds[handle];
}
