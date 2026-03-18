/**
 * @file Audio.hpp
 * @brief Provides audio-related utilities for the Nebulite engine.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_AUDIO_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_AUDIO_HPP

//------------------------------------------
// Includes

// Standard Library
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Constants/StandardCapture.hpp"
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
    [[nodiscard]] Constants::Event update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    //------------------------------------------
    // Category names

    [[nodiscard]] Constants::Event beep(std::span<std::string const> const& args) const;
    static auto constexpr beep_name = "beep";
    static auto constexpr beep_desc = "Make a beep noise.\n"
        "If no waveform type is specified, defaults to sine.\n"
        "Multiple waveform types can be specified at once, in which case they will be played in the order they were specified.\n"
        "Unknown waveform types will be ignored, but will print an error message.\n"
        "\n"
        "Usage: beep [sine/square/triangle]\n";

    [[nodiscard]] Constants::Event playSound(std::span<std::string const> const& args);
    static auto constexpr playSound_name = "play-sound";
    static auto constexpr playSound_desc = "Play a sound from a file.\n"
        "Usage: play-sound <file-path>\n";

    [[nodiscard]] Constants::Event playSoundWithFilter(std::span<std::string const> const& args);
    static auto constexpr playSoundWithFilter_name = "play-sound-filtered";
    static auto constexpr playSoundWithFilter_desc = "Play a sound from a file with a filter applied to it.\n"
        "Usage: play-sound-filtered <file-path> <num-coefficients> <den-coefficients>\n"
        "The coefficients should be specified as comma-separated values, with no spaces. For example:\n"
        "play-sound-filter my_sound.wav 0.1,0.1 1.0,-0.9\n";

    [[nodiscard]] Constants::Event testFilter(std::span<std::string const> const& args) const ;
    static auto constexpr testFilter_name = "audio-debug test-filter";
    static auto constexpr testFilter_desc = "Test a filter by applying it to sample values and printing the results.\n"
        "Usage: test-filter <sample> <num-coefficients> <den-coefficients>\n"
        "The coefficients should be specified as comma-separated values, with no spaces. For example:\n"
        "test-filter -0.5,0,0.5,1,0.5,0,-0.5,-1 1 0.5,0.5\n";

    //------------------------------------------
    // Categories

    static auto constexpr audioDebug_name = "audio-debug";
    static auto constexpr audioDebug_desc = "Audio debugging functions.";

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
        BIND_FUNCTION(&Audio::playSoundWithFilter, playSoundWithFilter_name, playSoundWithFilter_desc);

        bindCategory(audioDebug_name, audioDebug_desc);
        BIND_FUNCTION(&Audio::testFilter, testFilter_name, testFilter_desc);

        initAudio();
        initWaveforms();
    }

private:
    SDL_AudioStream* stream = nullptr;
    SDL_AudioSpec spec = {};

    struct Settings {
        using SampleType = float;

        static SampleType constexpr SampleMax = std::is_floating_point_v<SampleType> ? static_cast<SampleType>( 1.0) : std::numeric_limits<SampleType>::max();
        static SampleType constexpr SampleMin = std::is_floating_point_v<SampleType> ? static_cast<SampleType>(-1.0) : std::numeric_limits<SampleType>::min();

        static double constexpr sampleRate = 44100.0;
    };

    struct BasicAudioWaveforms {
        struct Settings {
            static double constexpr frequency = 440.0; // 440 Hz beep
            static double constexpr duration = 200.0; // 200ms
            static size_t constexpr samples = static_cast<size_t>(Audio::Settings::sampleRate * duration / 1000.0); // Number of samples
        };

        std::array<Audio::Settings::SampleType, Settings::samples> sineBuffer{};
        std::array<Audio::Settings::SampleType, Settings::samples> squareBuffer{};
        std::array<Audio::Settings::SampleType, Settings::samples> triangleBuffer{};
    } basicAudioWaveforms{};

    struct Sound {
        // TODO: Either we add metadata or we reduce this to a type alias for std::vector<Settings::SampleType>
        std::vector<Settings::SampleType> audioData;
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

    /**
     * @brief Converts an SDL_AudioFormat to a human-readable string for error messages.
     * @param format The SDL_AudioFormat to convert.
     * @return A string representation of the audio format.
     */
    static std::string sdlAudioFormatToString(SDL_AudioFormat const& format) {
        switch (format) {
            case SDL_AUDIO_U8: return "Unsigned 8-bit";
            case SDL_AUDIO_S8: return "Signed 8-bit";
            case SDL_AUDIO_F32: return "Floating point 32 bit";
            case SDL_AUDIO_S16: return "Signed 16-bit";
            case SDL_AUDIO_S16BE: return "Signed 16-bit big-endian";
            case SDL_AUDIO_S32: return "Signed 32-bit";
            case SDL_AUDIO_S32BE: return "Signed 32-bit big-endian";
            case SDL_AUDIO_F32BE: return "Floating point 32-bit big-endian";
            case SDL_AUDIO_UNKNOWN: return "Unknown";
            default: std::unreachable();
        }
    }
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_AUDIO_HPP
