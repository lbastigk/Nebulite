//------------------------------------------
// Includes

// Standard Library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "DomainModule/Renderer/Audio.hpp"
#include "Math/FFT.hpp"
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
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const path = Utility::StringHandler::recombineArgs(args | std::views::drop(1));
    auto const sound = loadSound(path);
    if (!sound.has_value()) {
        Error::println("Failed to load sound from path: ", path);
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }

    SDL_PutAudioStreamData(
        stream,
        sound.value()->second.audioData.data(),
        static_cast<int>(sound.value()->second.audioData.size() * sizeof(Settings::SampleType))
    );

    return Constants::ErrorTable::NONE();
}

Constants::Error Audio::playSoundWithFilter(std::span<std::string const> const& args) {
    if (args.size() < 4) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const path = args[1];
    auto const sound = loadSound(path);
    if (!sound.has_value()) {
        Error::println("Failed to load sound from path: ", path);
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }

    auto const [num, den] = [&]() -> std::pair<std::vector<double>, std::vector<double>> {
        try {
            auto parse = [](std::string const& str) {
                return Utility::StringHandler::split(str, ',')
                    | std::views::transform([](std::string const& coeff) {
                        return std::stod(coeff);
                    })
                    | std::ranges::to<std::vector<double>>();
            };

            return { parse(args[2]), parse(args[3]) };
        }
        catch (std::exception const& e) {
            Error::println("Failed to parse coefficients: ", e.what());
            throw;
        }
    }();

    auto const data = Math::FFT::applyTransferFunction(
        sound.value()->second.audioData | std::views::transform([](Settings::SampleType const& sample) {
            return static_cast<double>(sample);
        }) | std::ranges::to<std::vector>(),
        num,
        den
    ) | std::views::transform([](double const& sample) {
        return static_cast<float>(sample);
    }) | std::ranges::to<std::vector>();

    SDL_PutAudioStreamData(
        stream,
        data.data(),
        static_cast<int>(data.size() * sizeof(Settings::SampleType))
    );

    return Constants::ErrorTable::NONE();
}

Constants::Error Audio::testFilter(std::span<std::string const> const& args){
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    auto const data = Utility::StringHandler::split(args[1], ',');
    auto const num = Utility::StringHandler::split(args[2], ',');
    auto const den = Utility::StringHandler::split(args[3], ',');

    std::vector<double> inputData;
    std::vector<double> numData;
    std::vector<double> denData;

    try {
        for (auto const& coeff : data) {
            inputData.push_back(std::stod(coeff));
        }
        for (auto const& coeff : num) {
            numData.push_back(std::stod(coeff));
        }
        for (auto const& coeff : den) {
            denData.push_back(std::stod(coeff));
        }
    } catch (std::exception const& e) {
        Error::println("Failed to parse coefficients: ", e.what());
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }

    auto const out = Math::FFT::applyTransferFunction({inputData}, numData, denData);
    Log::println("Output:");
    for (auto const& sample : out) {
        Log::println(sample);
    }
    return Constants::ErrorTable::NONE();
}

void Audio::initAudio(){
    // Init
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        Error::println("SDL_Init Audio Error: ", SDL_GetError());
        std::abort();
    }
    spec.freq = 44100;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1; // TODO: Set default to stereo, convert all mono sounds to stereo. How are multiple channels handled in the audio stream?

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream) {
        Error::println("Failed to open audio device: ", SDL_GetError());
        std::abort();
    }
    SDL_ResumeAudioStreamDevice(stream);
}

void Audio::initWaveforms() {
    static_assert(!std::is_unsigned_v<Settings::SampleType>, "SampleType must be a signed type");
    static double constexpr amplitudeScale = 0.3 * static_cast<double>(Settings::SampleMax); // Scale down the amplitude to prevent clipping

    using SampleType = Settings::SampleType;
    static auto constexpr SampleCount = BasicAudioWaveforms::Settings::samples;
    static auto constexpr frequency = static_cast<double>(BasicAudioWaveforms::Settings::frequency);

    // NOLINTNEXTLINE
    auto constexpr time = [](std::size_t const& i) {
        return static_cast<double>(i) / Settings::sampleRate;
    };

    basicAudioWaveforms.sineBuffer = Utility::Generate::array<SampleType, SampleCount>([time](std::size_t const& i) {
        return static_cast<Settings::SampleType>(amplitudeScale * sin(2.0 * M_PI * frequency * time(i)));
    });
    basicAudioWaveforms.triangleBuffer = Utility::Generate::array<SampleType, SampleCount>([time](std::size_t const& i) {
        double const value = 2.0 * (time(i) * frequency - floor(time(i) * frequency + 0.5));
        return static_cast<Settings::SampleType>(amplitudeScale * value);
    });
    basicAudioWaveforms.squareBuffer = Utility::Generate::array<SampleType, SampleCount>([time](std::size_t const& i) {
        double const value = sin(2.0 * M_PI * frequency * time(i)) >= 0.0 ? 1.0 : -1.0;
        return static_cast<Settings::SampleType>(amplitudeScale * value);
    });
}

std::optional<decltype(Audio::soundCache.find(""))> Audio::loadSound(std::string const& path) {
    if (auto const it = soundCache.find(path); it != soundCache.end()) {
        return it;
    }

    Uint8* data = nullptr;
    Uint32 length = 0;
    SDL_AudioSpec wavSpec = {};
    SDL_LoadWAV(path.c_str(), &wavSpec, &data, &length);
    if (!data || length == 0) {
        Error::println("SDL_LoadWAV Error: ", SDL_GetError());
        return std::nullopt;
    }

    // Check if sound is the correct format
    if (wavSpec.channels != spec.channels || wavSpec.freq != spec.freq) {
        Error::println("Sound format does not match audio stream format. Sound: ", path);
        // TODO: Implement channel and sample rate conversion
        //       Probably best to do this after the conversion to float
        return std::nullopt;
    }

    // Push sound data into cache
    Sound sound;
    size_t lengthPerSample = 0;
    std::function<Settings::SampleType(Uint8* data)> convertFunc = nullptr;
    switch (wavSpec.format) {
        case SDL_AUDIO_F32:
            lengthPerSample = sizeof(float);
            convertFunc = [](Uint8 const* byteData) {
                float buffer[4];
                std::memcpy(buffer, byteData, sizeof(float));
                return *reinterpret_cast<float*>(buffer);
            };
            break;
        case SDL_AUDIO_S16:
            lengthPerSample = sizeof(int16_t);
            convertFunc = [](Uint8 const* byteData) {
                int16_t buffer[4];
                std::memcpy(buffer, byteData, sizeof(int16_t));
                return static_cast<Settings::SampleType>(*reinterpret_cast<int16_t*>(buffer)) / static_cast<Settings::SampleType>(std::numeric_limits<int16_t>::max());
            };
            break;
        case SDL_AUDIO_U8:
            lengthPerSample = sizeof(uint8_t);
            convertFunc = [](Uint8 const* byteData) {
                uint8_t buffer[4];
                std::memcpy(buffer, byteData, sizeof(uint8_t));
                return static_cast<Settings::SampleType>(*reinterpret_cast<uint8_t*>(buffer) - 128) / static_cast<Settings::SampleType>(std::numeric_limits<uint8_t>::max() / 2);
            };
            break;
        case SDL_AUDIO_S8:
        case SDL_AUDIO_S16BE:
        case SDL_AUDIO_S32:
        case SDL_AUDIO_S32BE:
        case SDL_AUDIO_F32BE:
        case SDL_AUDIO_UNKNOWN:
            Error::println("Unsupported audio format: ", sdlAudioFormatToString(wavSpec.format), " for sound: ", path, ". Feel free to submit a PR to add support for this format in function: ", __func__);
            SDL_free(data);
            return std::nullopt;
        default:
            std::unreachable();
    }

    size_t const sampleCount = length / lengthPerSample;
    sound.audioData.reserve(sampleCount);
    for (size_t i = 0; i < sampleCount; ++i) {
        sound.audioData.push_back(convertFunc(data + i * lengthPerSample));
    }

    soundCache.emplace(path, sound);
    SDL_free(data);
    auto const it = soundCache.find(path);
    if (it == soundCache.end()) {
        Error::println("Failed to cache sound after loading: ", path);
        return std::nullopt;
    }
    return it;
}

} // namespace Nebulite::DomainModule::Renderer
