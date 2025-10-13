#pragma once

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Plugins/Plugin.hpp>
#include <R-Engine/Types.hpp>

#include <string>
#include <unordered_map>

namespace r {

using AudioHandle = u32;
static constexpr inline const AudioHandle AudioInvalidHandle = static_cast<AudioHandle>(-1);

/**
 * @brief AudioManager loads and manages audio resources
 */
class R_ENGINE_API AudioManager final
{
    public:
        AudioManager() noexcept = default;
        ~AudioManager();

        /**
         * @brief Load an audio resource from the specified file path.
         * @return AudioHandle to the loaded resource, or AudioInvalidHandle on failure.
         */
        AudioHandle load(const std::string &path);

        /**
         * @brief Unload the audio resource associated with the given handle.
         * @details safe to call multiple times
         * @param handle The AudioHandle of the resource to unload.
         */
        void unload(const AudioHandle handle) noexcept;

        /**
         * @brief Get raw RayLib Sound *ptr
         */
        const ::Sound *get(AudioHandle handle) const noexcept;

    private:
        std::unordered_map<std::string, AudioHandle> _audios;
        std::vector<::Sound> _sounds;
};

/**
 * @brief AudioSource component
 * @details Lightweight handle referencing an audio resource.
 */
struct R_ENGINE_API AudioPlayer {
    public:
        AudioPlayer() = default;
        explicit AudioPlayer(const AudioHandle h) noexcept;

        AudioHandle id = AudioInvalidHandle;
        bool looping = false;
};

/**
 * @brief Controller component for runtime control.
 */
struct R_ENGINE_API AudioSink {
    public:
        f32 volume = 1.f;
        f32 pitch = 1.f;
        bool paused = false;
        bool muted = false;
};

/**
 * @brief Audio Plugin registering audio systems & manager.
 */
class R_ENGINE_API AudioPlugin final : public Plugin
{
    public:
        AudioPlugin() = default;
        ~AudioPlugin() override = default;

        void build(Application &app) override;
};

}// namespace r
