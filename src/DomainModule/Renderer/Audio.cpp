#include "Nebulite.hpp"
#include "DomainModule/Renderer/Audio.hpp"

namespace Nebulite::DomainModule::Renderer {

Constants::Error Audio::update() {
    // No update logic needed for now, as we're using SDL's audio stream to manage playback
    return Constants::ErrorTable::NONE();
}

Constants::Error Audio::beep() const {
    // SDL3: use an SDL_AudioStream to enqueue PCM data (lazy-initialized)
    static SDL_AudioStream* s_beepStream = nullptr;
    SDL_SetAudioStreamFormat(s_beepStream, &audio.desired, &audio.desired);
    int const audioLength = static_cast<int>(basicAudioWaveforms.samples * sizeof(int16_t));

    if (!s_beepStream) {
        s_beepStream = SDL_CreateAudioStream(&audio.desired, &audio.desired);
        if (!s_beepStream) {
            Error::println(SDL_GetError());
            return Constants::ErrorTable::RENDERER::AUDIO::CRITICAL_AUDIO_STREAM_CREATION_FAILED();
        }
    }

    if (SDL_PutAudioStreamData(s_beepStream, basicAudioWaveforms.squareBuffer->data(), audioLength) != 0) {
        // TODO: Parameter 'stream' is invalid
        Error::println(SDL_GetError());
        return Constants::ErrorTable::RENDERER::AUDIO::CRITICAL_AUDIO_STREAM_PUSH_FAILED();
    }

    // Ensure the device is running; SDL3's pause API uses a single-argument form
    SDL_PauseAudioDevice(audio.device);
    return Constants::ErrorTable::NONE();
}

void Audio::initAudio(){
    // Init
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        Error::println("SDL_Init Audio Error: ", SDL_GetError());
        std::abort();
    }
    audio.desired.freq = 44100;
    audio.desired.format = SDL_AUDIO_S16;
    audio.desired.channels = 1;
    //audio.desired.samples = 1024;
    //audio.desired.callback = nullptr;

    audio.device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio.desired);
    if (audio.device == 0) {
        Error::println("Failed to open audio device: ", SDL_GetError());
        std::abort();
    }
}

void Audio::initWaveforms() {
    // Waveform buffers: Sine wave buffer
    basicAudioWaveforms.sineBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
    for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
        double const time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;
        (*basicAudioWaveforms.sineBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * sin(2.0 * M_PI * basicAudioWaveforms.frequency * time));
    }

    // Waveform buffers: Square wave buffer
    basicAudioWaveforms.squareBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
    for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
        double const time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;

        // Square wave: alternates between +1 and -1
        double const phase = 2.0 * M_PI * basicAudioWaveforms.frequency * time;
        double const squareValue = sin(phase) >= 0 ? 1.0 : -1.0;

        (*basicAudioWaveforms.squareBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * squareValue);
    }

    // Waveform buffers: Triangle wave buffer
    basicAudioWaveforms.triangleBuffer = new std::vector<Sint16>(basicAudioWaveforms.samples);
    for (size_t i = 0; i < basicAudioWaveforms.samples; i++) {
        double const time = static_cast<double>(i) / basicAudioWaveforms.sampleRate;

        // Triangle wave: linear ramp up and down
        double const phase = fmod(basicAudioWaveforms.frequency * time, 1.0); // 0 to 1
        double triangleValue;

        if (phase < 0.5) {
            triangleValue = 4.0 * phase - 1.0; // -1 to +1 (rising)
        } else {
            triangleValue = 3.0 - 4.0 * phase; // +1 to -1 (falling)
        }

        (*basicAudioWaveforms.triangleBuffer)[i] = static_cast<int16_t>(32767 * 0.3 * triangleValue);
    }
}

} // namespace Nebulite::DomainModule::Renderer