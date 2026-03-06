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

    Constants::Error beep() const;
    static auto constexpr beep_name = "beep";
    static auto constexpr beep_desc = "Make a beep noise.\n"
        "\n"
        "Usage: beep\n";

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

        initAudio();
        initWaveforms();
    }

private:
    struct AudioSpec {
        SDL_AudioDeviceID device = 0;
        SDL_AudioSpec desired = {};
        SDL_AudioSpec obtained = {};
    } audio;

    struct BasicAudioWaveforms {
        double const frequency = 440.0; // 440 Hz beep
        double const duration = 200.0; // 200ms
        double const sampleRate = 44100.0;
        size_t const samples = static_cast<size_t>(sampleRate * duration / 1000.0); // Number of samples
        std::vector<int16_t>* sineBuffer = nullptr;
        std::vector<int16_t>* squareBuffer = nullptr;
        std::vector<int16_t>* triangleBuffer = nullptr;
    } basicAudioWaveforms;

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
