/**
 * @file StandardCapture.hpp
 * @brief
 */

#ifndef NEBULITE_CONSTANTS_STANDARD_CAPTURE_HPP
#define NEBULITE_CONSTANTS_STANDARD_CAPTURE_HPP

#include "Utility/IO/Capture.hpp"

namespace Nebulite::Constants {

enum class Event : uint8_t {
    Success = 0,
    Warning,
    Error
};

/**
 * @struct Nebulite::Constants::StandardCapture
 * @brief Standard capture outputs for various warnings, errors, etc.
 * @details The advantage of these standard errors is the reusability in multiple places.
 *          Takes care of both the logging and the proper Event return value.
 */
struct StandardCapture {

    struct Warning {
        struct Renderer {
            static Event snapshotFailed(Utility::IO::Capture& cap) {
                cap.warning.println("Renderer snapshot failed.");
                return Event::Warning;
            }
        };

        struct Functional {
            static Event functionCallInvalid(Utility::IO::Capture& cap) {
                cap.warning.println("Requested function call is invalid.");
                return Event::Warning;
            }

            static Event invalidArgcArgvParsing(Utility::IO::Capture& cap) {
                cap.warning.println("argc/argv parsing error.");
                return Event::Warning;
            }

            static Event tooManyArgs(Utility::IO::Capture& cap) {
                cap.warning.println("Too Many Arguments in function call");
                return Event::Warning;
            }

            static Event tooFewArgs(Utility::IO::Capture& cap) {
                cap.warning.println("Too Few Arguments in function call");
                return Event::Warning;
            }

            static Event unknownArg(Utility::IO::Capture& cap) {
                cap.warning.println("Unknown Argument Error");
                return Event::Warning;
            }

            static Event featureNotImplemented(Utility::IO::Capture& cap) {
                cap.warning.println("Requested feature of functioncall is not implemented");
                return Event::Warning;
            }

            static Event bindingCollision(Utility::IO::Capture& cap) {
                cap.warning.println("Binding Collision: Failed to bind a function, category or variable.");
                return Event::Warning;
            }

            static Event bindingCollisionExpected(Utility::IO::Capture& cap) {
                cap.warning.println("Binding Collision Expected: No collision occurred when one was expected.");
                return Event::Warning;
            }
        };
    };

    struct Error {
        struct SDL {
            static Event initFailed(Utility::IO::Capture& cap) {
                cap.error.println("SDL Renderer could not be initialized. This may cause rendering issues.");
                return Event::Error;
            }
            static Event settingTargetFailed(Utility::IO::Capture& cap) {
                cap.error.println("SDL Renderer target could not be set.");
                return Event::Error;
            }
        };

        struct Renderer {
            static Event notInitialized(Utility::IO::Capture& cap) {
                cap.error.println("Renderer not initialized.");
                return Event::Error;
            }
        };

        struct Texture {
            static Event notFound(Utility::IO::Capture& cap) {
                cap.error.println("Texture not found.");
                return Event::Error;
            }

            static Event copyFailed(Utility::IO::Capture& cap) {
                cap.error.println("Texture copy failed.");
                return Event::Error;
            }

            static Event colorUnsupported(Utility::IO::Capture& cap) {
                cap.error.println("Texture color format unsupported.");
                return Event::Error;
            }

            static Event lockFailed(Utility::IO::Capture& cap) {
                cap.error.println("Texture lock failed.");
                return Event::Error;
            }

            static Event queryFailed(Utility::IO::Capture& cap) {
                cap.error.println("Texture query failed.");
                return Event::Error;
            }

            static Event modificationFailed(Utility::IO::Capture& cap) {
                cap.error.println("Texture modification failed.");
                return Event::Error;
            }

            static Event invalid(Utility::IO::Capture& cap) {
                cap.error.println("Texture is invalid.");
                return Event::Error;
            }
        };

        struct Audio {
            static Event deviceInitFailed(Utility::IO::Capture& cap) {
                cap.error.println("Audio device could not be initialized.");
                return Event::Error;
            }
        };

        struct Functional {
            static Event functionNotImplemented(Utility::IO::Capture& cap) {
                cap.error.println("Requested function not implemented.");
                return Event::Error;
            }
        };

        struct File {
            static Event invalidFile(Utility::IO::Capture& cap) {
                cap.error.println("Requested file is invalid.");
                return Event::Error;
            }
            static Event couldNotWriteFile(Utility::IO::Capture& cap) {
                cap.error.println("Couldn't write to file.");
                return Event::Error;
            }
        };

        struct Ruleset {
            static Event parsingFailed(Utility::IO::Capture& cap) {
                cap.error.println("Ruleset parsing failed.");
                return Event::Error;
            }
        };
    };
};
} // namespace Nebulite::Constants
#endif // NEBULITE_CONSTANTS_STANDARD_CAPTURE_HPP
