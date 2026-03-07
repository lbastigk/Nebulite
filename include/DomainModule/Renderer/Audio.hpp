/**
 * @file Audio.hpp
 * @brief Provides audio-related utilities for the Nebulite engine.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_AUDIO_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_AUDIO_HPP

//------------------------------------------
// Includes

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer; // Forward declaration of domain class Renderer
class RenderObject; // Forward declaration of RenderObject
}

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Audio
 * @brief Basic Renderer-Related Functions
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Audio) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    //------------------------------------------
    // Category names

    Constants::Error beep(std::span<std::string const> const& args) const;
    static auto constexpr beep_name = "beep";
    static auto constexpr beep_desc = "Make a beep noise.\n"
        "If no waveform type is specified, defaults to sine.\n"
        "Multiple waveform types can be specified at once, in which case they will be played in the order they were specified.\n"
        "Unknown waveform types will be ignored, but will print an error message.\n"
        "\n"
        "Usage: beep [sine/square/triangle]\n";

    Constants::Error playSound(std::span<std::string const> const& args);
    static auto constexpr playSound_name = "play-sound";
    static auto constexpr playSound_desc = "Play a sound from a file.\n"
        "Usage: play-sound <file-path>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     * @todo Add domainModules for camera and renderobject-selection and move respective functions in there.
     * @todo Move functions for Renderer and Environment to domains themselves,
     *       once they are implemented as such.
     *       This will declutter the globalspace, separating its usage from the Renderer and Environment.
     *       The only downside currently is that we have to implement a method to lazy-init the SDL Renderer within the Renderer domain itself.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Audio) {
        BIND_FUNCTION(&Audio::beep, beep_name, beep_desc);
        BIND_FUNCTION(&Audio::playSound, playSound_name, playSound_desc);

        initAudio();
        initWaveforms();
    }

private:
    SDL_AudioStream* stream = nullptr;
    SDL_AudioSpec spec = {};

    struct BasicAudioWaveforms {
        struct Settings {
            using SampleType = float;

            static SampleType constexpr SampleMax = std::is_floating_point_v<SampleType> ? static_cast<SampleType>( 1.0) : std::numeric_limits<SampleType>::max();
            static SampleType constexpr SampleMin = std::is_floating_point_v<SampleType> ? static_cast<SampleType>(-1.0) : std::numeric_limits<SampleType>::min();

            static double constexpr frequency = 440.0; // 440 Hz beep
            static double constexpr duration = 200.0; // 200ms
            static double constexpr sampleRate = 44100.0;
            static size_t constexpr samples = static_cast<size_t>(sampleRate * duration / 1000.0); // Number of samples
        };

        std::array<Settings::SampleType, Settings::samples> sineBuffer;
        std::array<Settings::SampleType, Settings::samples> squareBuffer;
        std::array<Settings::SampleType, Settings::samples> triangleBuffer;
    } basicAudioWaveforms;

    struct Sound {
        uint8_t* buffer;
        uint32_t length;
        SDL_AudioSpec spec;
    };

    absl::flat_hash_map<std::string, Sound> soundCache;

    std::optional<decltype(soundCache.find(""))> loadSound(std::string const& path);

    /**
     * @brief Initializes basic audio waveforms.
     */
    void initWaveforms();

    /**
     * @brief Initializes SDL audio subsystem and opens the audio device.
     */
    void initAudio();
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_AUDIO_HPP
