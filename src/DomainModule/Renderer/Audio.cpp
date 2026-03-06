//------------------------------------------
// Includes

// Standard Library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "DomainModule/Renderer/Audio.hpp"
#include "Utility/Generate.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Audio::update() {
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
    static double constexpr amplitudeScale = 0.3 * BasicAudioWaveforms::Settings::SampleMax; // Scale down the amplitude to prevent clipping

    using SampleRange = BasicAudioWaveforms::Settings::SampleRange;
    static auto constexpr SampleCount = BasicAudioWaveforms::Settings::samples;

    // NOLINTNEXTLINE
    auto constexpr time = [](std::size_t const& i) {
        return static_cast<double>(i) / BasicAudioWaveforms::Settings::sampleRate;
    };

    basicAudioWaveforms.sineBuffer = Utility::Generate::array<SampleRange, SampleCount>([time](std::size_t const& i) {
        return static_cast<int16_t>(amplitudeScale * sin(2.0 * M_PI * BasicAudioWaveforms::Settings::frequency * time(i)));
    });
    basicAudioWaveforms.triangleBuffer = Utility::Generate::array<SampleRange, SampleCount>([time](std::size_t const& i) {
        double const value = 2.0 * (time(i) * BasicAudioWaveforms::Settings::frequency - floor(time(i) * BasicAudioWaveforms::Settings::frequency + 0.5));
        return static_cast<int16_t>(amplitudeScale * value);
    });
    basicAudioWaveforms.squareBuffer = Utility::Generate::array<SampleRange, SampleCount>([time](std::size_t const& i) {
        double const value = sin(2.0 * M_PI * BasicAudioWaveforms::Settings::frequency * time(i)) >= 0.0 ? 1.0 : -1.0;
        return static_cast<int16_t>(amplitudeScale * value);
    });
}

} // namespace Nebulite::DomainModule::Renderer
