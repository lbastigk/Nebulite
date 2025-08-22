#pragma once

/*
================================================================================
                           AUDIO EXPANSION - IDEAS PHASE
================================================================================

⚠️  WARNING: This expansion is currently in the IDEAS PHASE and NOT IMPLEMENTED

This header file contains the proposed interface for the Nebulite Audio 
expansion system. None of the functions declared here have been implemented yet.

The Audio expansion is designed to provide comprehensive audio management 
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

TODO: Once merged into the GlobalSpace, please move GTE_Renderer:beep to GTE_Audio:beep

================================================================================
*/

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace

namespace GlobalSpaceTreeExpansion {
class Audio : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, Audio> {
public:
    using Wrapper<Nebulite::GlobalSpace, Audio>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions

    // Basic audio playback
    Nebulite::ERROR_TYPE playSound(int argc, char* argv[]);        // Play sound effect: play-sound <file> [volume] [pan]
    Nebulite::ERROR_TYPE playMusic(int argc, char* argv[]);        // Play background music: play-music <file> [volume] [loop]
    Nebulite::ERROR_TYPE stopSound(int argc, char* argv[]);        // Stop specific sound: stop-sound <id>
    Nebulite::ERROR_TYPE stopMusic(int argc, char* argv[]);        // Stop music: stop-music
    Nebulite::ERROR_TYPE stopAll(int argc, char* argv[]);          // Stop all audio: stop-all

    // Volume and mixing controls
    Nebulite::ERROR_TYPE setMasterVolume(int argc, char* argv[]);  // Set master volume: set-master-volume <0.0-1.0>
    Nebulite::ERROR_TYPE setSfxVolume(int argc, char* argv[]);     // Set sound effects volume: set-sfx-volume <0.0-1.0>
    Nebulite::ERROR_TYPE setMusicVolume(int argc, char* argv[]);   // Set music volume: set-music-volume <0.0-1.0>
    Nebulite::ERROR_TYPE fadeIn(int argc, char* argv[]);           // Fade in audio: fade-in <file> <duration_ms> [volume]
    Nebulite::ERROR_TYPE fadeOut(int argc, char* argv[]);          // Fade out audio: fade-out <id> <duration_ms>

    // Advanced audio features
    Nebulite::ERROR_TYPE setPan(int argc, char* argv[]);           // Set stereo panning: set-pan <id> <-1.0 to 1.0>
    Nebulite::ERROR_TYPE setPitch(int argc, char* argv[]);         // Set playback pitch: set-pitch <id> <pitch_factor>
    Nebulite::ERROR_TYPE setPosition3D(int argc, char* argv[]);    // Set 3D audio position: set-position-3d <id> <x> <y> <z>
    Nebulite::ERROR_TYPE setListener3D(int argc, char* argv[]);    // Set 3D listener position: set-listener-3d <x> <y> <z>

    // Procedural audio generation (extending the existing beep functionality)
    Nebulite::ERROR_TYPE generateTone(int argc, char* argv[]);     // Generate tone: generate-tone <frequency> <duration_ms> [waveform] [volume]
    Nebulite::ERROR_TYPE generateNoise(int argc, char* argv[]);    // Generate noise: generate-noise <type> <duration_ms> [volume]
    Nebulite::ERROR_TYPE createWaveform(int argc, char* argv[]);   // Create custom waveform: create-waveform <name> <samples...>

    // Audio recording and capture
    Nebulite::ERROR_TYPE startRecording(int argc, char* argv[]);   // Start audio recording: start-recording [filename]
    Nebulite::ERROR_TYPE stopRecording(int argc, char* argv[]);    // Stop audio recording: stop-recording
    Nebulite::ERROR_TYPE captureOutput(int argc, char* argv[]);    // Capture engine audio output: capture-output <filename> <duration_ms>

    // Audio analysis and visualization
    Nebulite::ERROR_TYPE analyzeSpectrum(int argc, char* argv[]);  // Analyze audio spectrum: analyze-spectrum <id>
    Nebulite::ERROR_TYPE getVolumeLevels(int argc, char* argv[]);  // Get current volume levels: get-volume-levels
    Nebulite::ERROR_TYPE detectBeat(int argc, char* argv[]);       // Beat detection: detect-beat <id> [sensitivity]

    // Audio device management
    Nebulite::ERROR_TYPE listDevices(int argc, char* argv[]);      // List audio devices: list-devices
    Nebulite::ERROR_TYPE setOutputDevice(int argc, char* argv[]);  // Set output device: set-output-device <device_id>
    Nebulite::ERROR_TYPE setInputDevice(int argc, char* argv[]);   // Set input device: set-input-device <device_id>

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        // Basic playback
        bindFunction(&Audio::playSound,         "play-sound",           "Play sound effect");
        bindFunction(&Audio::playMusic,         "play-music",           "Play background music");
        bindFunction(&Audio::stopSound,         "stop-sound",           "Stop specific sound");
        bindFunction(&Audio::stopMusic,         "stop-music",           "Stop music");
        bindFunction(&Audio::stopAll,           "stop-all",             "Stop all audio");

        // Volume controls
        bindFunction(&Audio::setMasterVolume,   "set-master-volume",    "Set master volume");
        bindFunction(&Audio::setSfxVolume,      "set-sfx-volume",       "Set sound effects volume");
        bindFunction(&Audio::setMusicVolume,    "set-music-volume",     "Set music volume");
        bindFunction(&Audio::fadeIn,            "fade-in",              "Fade in audio");
        bindFunction(&Audio::fadeOut,           "fade-out",             "Fade out audio");

        // Advanced features
        bindFunction(&Audio::setPan,            "set-pan",              "Set stereo panning");
        bindFunction(&Audio::setPitch,          "set-pitch",            "Set playback pitch");
        bindFunction(&Audio::setPosition3D,     "set-position-3d",      "Set 3D audio position");
        bindFunction(&Audio::setListener3D,     "set-listener-3d",      "Set 3D listener position");

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
}   // namespace GlobalSpaceTreeExpansion
}   // namespace Nebulite
