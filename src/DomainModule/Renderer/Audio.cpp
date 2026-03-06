#include "Nebulite.hpp"
#include "DomainModule/Renderer/Audio.hpp"
#include "Utility/Generate.hpp"

namespace Nebulite::DomainModule::Renderer {

Constants::Error Audio::update() {
    // No update logic needed for now, as we're using SDL's audio stream to manage playback
    return Constants::ErrorTable::NONE();
}

Constants::Error Audio::beep() const {
    // SDL3: use an SDL_AudioStream to enqueue PCM data (lazy-initialized)
    static SDL_AudioStream* s_beepStream = nullptr;
    SDL_SetAudioStreamFormat(s_beepStream, &audio.desired, &audio.desired);
    int constexpr audioLength = BasicAudioWaveforms::Settings::samples * sizeof(int16_t);

    if (!s_beepStream) {
        s_beepStream = SDL_CreateAudioStream(&audio.desired, &audio.desired);
        if (!s_beepStream) {
            Error::println(SDL_GetError());
            return Constants::ErrorTable::RENDERER::AUDIO::CRITICAL_AUDIO_STREAM_CREATION_FAILED();
        }
    }

    if (SDL_PutAudioStreamData(s_beepStream, basicAudioWaveforms.squareBuffer.data(), audioLength) != 0) {
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

    audio.device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio.desired);
    if (audio.device == 0) {
        Error::println("Failed to open audio device: ", SDL_GetError());
        std::abort();
    }
}

void Audio::initWaveforms() {
    static_assert(!std::is_unsigned_v<BasicAudioWaveforms::Settings::SampleRange>, "SampleRange must be a signed type");

    // Waveform buffers: Sine wave buffer
    basicAudioWaveforms.sineBuffer = Utility::Generate::array<BasicAudioWaveforms::Settings::SampleRange, BasicAudioWaveforms::Settings::samples>([](std::size_t const& i) {
        double const time = static_cast<double>(i) / BasicAudioWaveforms::Settings::sampleRate;
        return static_cast<int16_t>(BasicAudioWaveforms::Settings::SampleMax * 0.3 * sin(2.0 * M_PI * BasicAudioWaveforms::Settings::frequency * time));
    });
    basicAudioWaveforms.triangleBuffer = Utility::Generate::array<BasicAudioWaveforms::Settings::SampleRange, BasicAudioWaveforms::Settings::samples>([](std::size_t const& i) {
        double const time = static_cast<double>(i) / BasicAudioWaveforms::Settings::sampleRate;
        double const value = 2.0 * (time * BasicAudioWaveforms::Settings::frequency - floor(time * BasicAudioWaveforms::Settings::frequency + 0.5));
        return static_cast<int16_t>(BasicAudioWaveforms::Settings::SampleMax * 0.3 * value);
    });
    basicAudioWaveforms.squareBuffer = Utility::Generate::array<BasicAudioWaveforms::Settings::SampleRange, BasicAudioWaveforms::Settings::samples>([](std::size_t const& i) {
        double const time = static_cast<double>(i) / BasicAudioWaveforms::Settings::sampleRate;
        double const value = sin(2.0 * M_PI * BasicAudioWaveforms::Settings::frequency * time) >= 0.0 ? 1.0 : -1.0;
        return static_cast<int16_t>(BasicAudioWaveforms::Settings::SampleMax * 0.3 * value);
    });
}

} // namespace Nebulite::DomainModule::Renderer
