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

Constants::Error Audio::beep(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        Log::println("No waveform type specified. Defaulting to sine.");
        SDL_PutAudioStreamData(
            stream,
            basicAudioWaveforms.sineBuffer.data(),
            static_cast<int>(basicAudioWaveforms.sineBuffer.size() * sizeof(int16_t))
        );
        return Constants::ErrorTable::NONE();
    }

    for (auto const& arg : args | std::views::drop(1)) {
        if (arg == "sine") {
            SDL_PutAudioStreamData(
                stream,
                basicAudioWaveforms.sineBuffer.data(),
                static_cast<int>(basicAudioWaveforms.sineBuffer.size() * sizeof(int16_t))
            );
        } else if (arg == "triangle") {
            SDL_PutAudioStreamData(
                stream,
                basicAudioWaveforms.triangleBuffer.data(),
                static_cast<int>(basicAudioWaveforms.triangleBuffer.size() * sizeof(int16_t))
            );
        } else if (arg == "square") {
            SDL_PutAudioStreamData(
                stream,
                basicAudioWaveforms.squareBuffer.data(),
                static_cast<int>(basicAudioWaveforms.squareBuffer.size() * sizeof(int16_t))
            );
        } else {
            Error::println("Unknown waveform type: ", arg);
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Audio::playSound(std::span<std::string const> const& args) {
    auto const path = Utility::StringHandler::recombineArgs(args | std::views::drop(1));
    auto const [data, length] = loadSound(path);
    if (!data || length == 0) {
        Error::println("Failed to load sound from path: ", path);
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }
    SDL_PutAudioStreamData(
        stream,
        data,
        static_cast<int>(length * sizeof(int16_t))
    );
    return Constants::ErrorTable::NONE();
}

void Audio::initAudio(){
    // Init
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        Error::println("SDL_Init Audio Error: ", SDL_GetError());
        std::abort();
    }
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream) {
        Error::println("Failed to open audio device: ", SDL_GetError());
        std::abort();
    }
    SDL_ResumeAudioStreamDevice(stream);
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

Audio::Sound Audio::loadSound(std::string const& path) {
    if (auto const it = soundCache.find(path); it != soundCache.end()) {
        return it->second;
    }

    Uint8* data = nullptr;
    Uint32 length = 0;
    SDL_LoadWAV(path.c_str(), &spec, &data, &length);
    if (!data || length == 0) {
        Error::println("SDL_LoadWAV Error: ", SDL_GetError());
        return {nullptr, 0};
    }
    soundCache[path] = {data, length};
    return {data, length};
}

} // namespace Nebulite::DomainModule::Renderer
