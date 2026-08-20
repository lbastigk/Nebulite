//------------------------------------------
// Includes

// Standard Library
#include <array>
#include <cstddef>
#include <functional>
#include <numbers>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// External
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_stdinc.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Math/ExpressionPrimitives.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Module/Domain/Renderer/Audio.hpp"
#include "Nebulite/Utility/Generate.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event Audio::updateHook() {
    return Constants::Event::success;
}

//------------------------------------------
// Available Functions

Constants::Event Audio::beep(std::span<std::string_view const> const args) const {
    if (args.size() < 2) {
        domain.capture.log.println("No waveform type specified. Defaulting to sine.");
        SDL_PutAudioStreamData(
            stream,
            basicAudioWaveforms.sineBuffer.data(),
            static_cast<int>(basicAudioWaveforms.sineBuffer.size() * sizeof(std::int16_t))
        );
        return Constants::Event::success;
    }

    for (auto const& arg : args | std::views::drop(1)) {
        if (arg == "sine") {
            SDL_PutAudioStreamData(
                stream,
                basicAudioWaveforms.sineBuffer.data(),
                static_cast<int>(basicAudioWaveforms.sineBuffer.size() * sizeof(std::int16_t))
            );
        } else if (arg == "triangle") {
            SDL_PutAudioStreamData(
                stream,
                basicAudioWaveforms.triangleBuffer.data(),
                static_cast<int>(basicAudioWaveforms.triangleBuffer.size() * sizeof(std::int16_t))
            );
        } else if (arg == "square") {
            SDL_PutAudioStreamData(
                stream,
                basicAudioWaveforms.squareBuffer.data(),
                static_cast<int>(basicAudioWaveforms.squareBuffer.size() * sizeof(std::int16_t))
            );
        } else {
            domain.capture.warning.println("Unknown waveform type: ", arg);
            return Constants::Event::warning;
        }
    }
    return Constants::Event::success;
}

Constants::Event Audio::playSound(std::span<std::string_view const> const args) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    auto const path = Utility::StringHandler::recombineArgs(args | std::views::drop(1));
    auto const sound = loadSound(path);
    if (!sound.has_value()) {
        domain.capture.error.println("Failed to load sound from path: ", path);
        return Constants::StandardCapture::Error::File::invalidFile(domain.capture);
    }

    // TODO: using SDL_PutAudioStreamData does not allow for multiple streams. We should switch to SDL_Mixer later on?
    //       for now we just play the latest sound
    SDL_ClearAudioStream(stream);
    SDL_PutAudioStreamData(
        stream,
        sound.value()->audioData.data(),
        static_cast<int>(sound.value()->audioData.size() * sizeof(Settings::SampleType))
    );

    return Constants::Event::success;
}

//------------------------------------------
// Setup

Audio::Audio(ConstructorParams const& params) : DomainModule(params) {
    bindFunction(&Audio::beep, beepName, beepDesc);
    bindFunction(&Audio::playSound, playSoundName, playSoundDesc);

    initAudio();
    initWaveforms();
}

//------------------------------------------
// Private functions

void Audio::initAudio(){
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        domain.capture.error.println("SDL_Init Audio Error: ", SDL_GetError());
        std::abort();
    }
    spec.freq = 44100;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1; // TODO: Set default to stereo, convert all mono sounds to stereo. How are multiple channels handled in the audio stream?

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream) {
        domain.capture.error.println("Failed to open audio device: ", SDL_GetError());
        std::abort();
    }
    SDL_ResumeAudioStreamDevice(stream);
}

void Audio::initWaveforms() {
    static_assert(!std::is_unsigned_v<Settings::SampleType>, "SampleType must be a signed type");
    static double constexpr amplitudeScale = 0.3 * static_cast<double>(Settings::SampleMax); // Scale down the amplitude to prevent clipping
    static auto constexpr omega = 2.0 * std::numbers::pi * BasicAudioWaveforms::Settings::frequency;

    basicAudioWaveforms.sineBuffer = Utility::Generate::array<Settings::SampleType, BasicAudioWaveforms::Settings::samples>(
        [](std::size_t const i) {
            return static_cast<Settings::SampleType>(amplitudeScale * sin(omega * timeAtSample(i)));
        }
    );
    basicAudioWaveforms.triangleBuffer = Utility::Generate::array<Settings::SampleType, BasicAudioWaveforms::Settings::samples>(
        [](std::size_t const i) {
            return static_cast<Settings::SampleType>(amplitudeScale * Math::ExpressionPrimitives::triangle(omega * timeAtSample(i)));
        }
    );
    basicAudioWaveforms.squareBuffer = Utility::Generate::array<Settings::SampleType, BasicAudioWaveforms::Settings::samples>(
        [](std::size_t const i) {
            return static_cast<Settings::SampleType>(amplitudeScale * Math::ExpressionPrimitives::square(omega * timeAtSample(i)));
        }
    );
}

std::optional<std::shared_ptr<Audio::Sound>> Audio::loadSound(std::string const& path){
    if (auto const it = soundCache.find(path); it != soundCache.end()) {
        return it->second;
    }

    Settings::SdlAudioByte* data = nullptr;
    std::uint32_t length = 0;
    SDL_AudioSpec wavSpec = {};
    SDL_LoadWAV(path.c_str(), &wavSpec, &data, &length);
    if (!data || length == 0) {
        domain.capture.error.println("SDL_LoadWAV Error: ", SDL_GetError());
        return std::nullopt;
    }

    // Check if sound is the correct format
    if (wavSpec.channels != spec.channels || wavSpec.freq != spec.freq) {
        domain.capture.error.println("Sound format does not match audio stream format. Sound: ", path);
        // TODO: Implement channel and sample rate conversion
        //       Probably best to do this after the conversion to float
        return std::nullopt;
    }

    // Push sound data into cache
    Sound sound;
    auto converterAndSampleSize = loadConverterFunction(wavSpec.format);
    if (!converterAndSampleSize) {
        domain.capture.error.println("Unsupported audio format: ", sdlAudioFormatToString(wavSpec.format), " for sound: ", path, ". Feel free to submit a PR to add support for this format in function: ", __FUNCTION__);
        SDL_free(data);
    }
    auto& [convertFunc, lengthPerSample] = converterAndSampleSize.value();
    auto const sampleCount = length / lengthPerSample;
    sound.audioData.reserve(sampleCount);
    for (std::size_t i = 0; i < sampleCount; ++i) {
        sound.audioData.push_back(convertFunc(data + i * lengthPerSample));
    }

    auto const soundPtr = std::make_shared<Sound>(sound);
    soundCache.emplace(path, soundPtr);
    SDL_free(data);
    auto const it = soundCache.find(path);
    if (it == soundCache.end()) {
        domain.capture.error.println("Failed to cache sound after loading: ", path);
        return std::nullopt;
    }
    return it->second;
}

std::string Audio::sdlAudioFormatToString(SDL_AudioFormat const format) {
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

std::optional<Audio::ConverterAndSampleSize> Audio::loadConverterFunction(SDL_AudioFormat const format) {
    switch (format) {
    case SDL_AUDIO_F32:
        return std::make_pair(
            [](Settings::SdlAudioByte const* byteData) {
                std::array<float,4> buffer{};
                std::memcpy(buffer.data(), byteData, sizeof(float));
                return *buffer.data();
            },
            sizeof(float)
        );
    case SDL_AUDIO_S16:
        return std::make_pair(
            [](Settings::SdlAudioByte const* byteData) {
                std::array<int16_t,4> buffer{};
                std::memcpy(buffer.data(), byteData, sizeof(std::int16_t));
                return static_cast<Settings::SampleType>(*buffer.data()) / static_cast<Settings::SampleType>(std::numeric_limits<int16_t>::max());
            },
            sizeof(std::int16_t)
        );
    case SDL_AUDIO_U8:
        return std::make_pair(
            [](Settings::SdlAudioByte const* byteData) {
                std::array<uint8_t,4> buffer{};
                std::memcpy(buffer.data(), byteData, sizeof(std::uint8_t));
                Settings::SampleType const valueShifted = static_cast<Settings::SampleType>(*buffer.data()) - static_cast<Settings::SampleType>(128);
                return valueShifted / (static_cast<Settings::SampleType>(std::numeric_limits<uint8_t>::max()) / static_cast<Settings::SampleType>(2));
            },
            sizeof(std::uint8_t)
        );
    case SDL_AUDIO_S8:
    case SDL_AUDIO_S16BE:
    case SDL_AUDIO_S32:
    case SDL_AUDIO_S32BE:
    case SDL_AUDIO_F32BE:
    case SDL_AUDIO_UNKNOWN:
        return std::nullopt;
    default:
        std::unreachable();
    }
}

} // namespace Nebulite::Module::Domain::Renderer
