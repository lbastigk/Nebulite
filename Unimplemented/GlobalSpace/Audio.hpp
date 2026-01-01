#pragma once

/*
================================================================================
                           AUDIO DomainModule - IDEAS PHASE
================================================================================

⚠️  WARNING: This DomainModule is currently in the IDEAS PHASE and NOT IMPLEMENTED

This header file contains the proposed interface for the Nebulite Audio 
DomainModule system. None of the functions declared here have been implemented yet.

The Audio DomainModule is designed to provide comprehensive audio management 
capabilities for the Nebulite engine, including:
- Basic audio playback and mixing
- Advanced 3D audio positioning
- Procedural audio generation
- Real-time audio analysis
- Recording and capture functionality
- Audio device management

Implementation Status: PLANNING PHASE ONLY
- Header declarations: Complete
- Function implementations: Not started
- SDL audio integration: Not started
- Testing: Not started

Do NOT attempt to use these functions until implementation is complete.

@todo: Once merged into the GlobalSpace, please move GDM_Renderer:beep to GDM_Audio:beep

================================================================================
*/

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Audio
 * @brief DomainModule for audio operations on domain class Nebulite::Core::GlobalSpace
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Audio){
public:
    //------------------------------------------
    // Available Functions

    // Basic audio playback
    Nebulite::Constants::Error playSound(int argc, char* argv[]);        // Play sound effect: play-sound <file> [volume] [pan]
    Nebulite::Constants::Error playMusic(int argc, char* argv[]);        // Play background music: play-music <file> [volume] [loop]
    Nebulite::Constants::Error stopSound(int argc, char* argv[]);        // Stop specific sound: stop-sound <id>
    Nebulite::Constants::Error stopMusic(int argc, char* argv[]);        // Stop music: stop-music
    Nebulite::Constants::Error stopAll(int argc, char* argv[]);          // Stop all audio: stop-all

    // Volume and mixing controls
    Nebulite::Constants::Error setMasterVolume(int argc, char* argv[]);  // Set master volume: set-master-volume <0.0-1.0>
    Nebulite::Constants::Error setSfxVolume(int argc, char* argv[]);     // Set sound effects volume: set-sfx-volume <0.0-1.0>
    Nebulite::Constants::Error setMusicVolume(int argc, char* argv[]);   // Set music volume: set-music-volume <0.0-1.0>
    Nebulite::Constants::Error fadeIn(int argc, char* argv[]);           // Fade in audio: fade-in <file> <duration_ms> [volume]
    Nebulite::Constants::Error fadeOut(int argc, char* argv[]);          // Fade out audio: fade-out <id> <duration_ms>

    // Advanced audio features
    Nebulite::Constants::Error setPan(int argc, char* argv[]);           // Set stereo panning: set-pan <id> <-1.0 to 1.0>
    Nebulite::Constants::Error setPitch(int argc, char* argv[]);         // Set playback pitch: set-pitch <id> <pitch_factor>
    Nebulite::Constants::Error setPosition3D(int argc, char* argv[]);    // Set 3D audio position: set-position-3d <id> <x> <y> <z>
    Nebulite::Constants::Error setListener3D(int argc, char* argv[]);    // Set 3D listener position: set-listener-3d <x> <y> <z>

    // Procedural audio generation (extending the existing beep functionality)
    Nebulite::Constants::Error generateTone(int argc, char* argv[]);     // Generate tone: generate-tone <frequency> <duration_ms> [waveform] [volume]
    Nebulite::Constants::Error generateNoise(int argc, char* argv[]);    // Generate noise: generate-noise <type> <duration_ms> [volume]
    Nebulite::Constants::Error createWaveform(int argc, char* argv[]);   // Create custom waveform: create-waveform <name> <samples...>

    // Audio recording and capture
    Nebulite::Constants::Error startRecording(int argc, char* argv[]);   // Start audio recording: start-recording [filename]
    Nebulite::Constants::Error stopRecording(int argc, char* argv[]);    // Stop audio recording: stop-recording
    Nebulite::Constants::Error captureOutput(int argc, char* argv[]);    // Capture engine audio output: capture-output <filename> <duration_ms>

    // Audio analysis and visualization
    Nebulite::Constants::Error analyzeSpectrum(int argc, char* argv[]);  // Analyze audio spectrum: analyze-spectrum <id>
    Nebulite::Constants::Error getVolumeLevels(int argc, char* argv[]);  // Get current volume levels: get-volume-levels
    Nebulite::Constants::Error detectBeat(int argc, char* argv[]);       // Beat detection: detect-beat <id> [sensitivity]

    // Audio device management
    Nebulite::Constants::Error listDevices(int argc, char* argv[]);      // List audio devices: list-devices
    Nebulite::Constants::Error setOutputDevice(int argc, char* argv[]);  // Set output device: set-output-device <device_id>
    Nebulite::Constants::Error setInputDevice(int argc, char* argv[]);   // Set input device: set-input-device <device_id>

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Audio){
        // Basic playback
        BINDFUNCTION(&Audio::playSound,         "play-sound",           "Play sound effect");
        BINDFUNCTION(&Audio::playMusic,         "play-music",           "Play background music");
        BINDFUNCTION(&Audio::stopSound,         "stop-sound",           "Stop specific sound");
        BINDFUNCTION(&Audio::stopMusic,         "stop-music",           "Stop music");
        BINDFUNCTION(&Audio::stopAll,           "stop-all",             "Stop all audio");

        // Volume controls
        BINDFUNCTION(&Audio::setMasterVolume,   "set-master-volume",    "Set master volume");
        BINDFUNCTION(&Audio::setSfxVolume,      "set-sfx-volume",       "Set sound effects volume");
        BINDFUNCTION(&Audio::setMusicVolume,    "set-music-volume",     "Set music volume");
        BINDFUNCTION(&Audio::fadeIn,            "fade-in",              "Fade in audio");
        BINDFUNCTION(&Audio::fadeOut,           "fade-out",             "Fade out audio");

        // Advanced features
        BINDFUNCTION(&Audio::setPan,            "set-pan",              "Set stereo panning");
        BINDFUNCTION(&Audio::setPitch,          "set-pitch",            "Set playback pitch");
        BINDFUNCTION(&Audio::setPosition3D,     "set-position-3d",      "Set 3D audio position");
        BINDFUNCTION(&Audio::setListener3D,     "set-listener-3d",      "Set 3D listener position");

        // Procedural generation
        bindFunction(&Audio::generateTone,      "generate-tone",        "Generate tone");
        bindFunction(&Audio::generateNoise,     "generate-noise",       "Generate noise");
        bindFunction(&Audio::createWaveform,    "create-waveform",      "Create custom waveform");

        // Recording
        bindFunction(&Audio::startRecording,    "start-recording",      "Start audio recording");
        bindFunction(&Audio::stopRecording,     "stop-recording",       "Stop audio recording");
        bindFunction(&Audio::captureOutput,     "capture-output",       "Capture engine audio output");

        // Analysis
        bindFunction(&Audio::analyzeSpectrum,   "analyze-spectrum",     "Analyze audio spectrum");
        bindFunction(&Audio::getVolumeLevels,   "get-volume-levels",    "Get current volume levels");
        bindFunction(&Audio::detectBeat,        "detect-beat",          "Beat detection");

        // Device management
        bindFunction(&Audio::listDevices,       "list-devices",         "List audio devices");
        bindFunction(&Audio::setOutputDevice,   "set-output-device",    "Set output device");
        bindFunction(&Audio::setInputDevice,    "set-input-device",     "Set input device");
    }
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite
