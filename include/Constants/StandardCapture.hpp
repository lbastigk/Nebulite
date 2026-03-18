/**
 * @file StandardCapture.hpp
 * @brief
 */

#ifndef NEBULITE_CONSTANTS_STANDARD_CAPTURE_HPP
#define NEBULITE_CONSTANTS_STANDARD_CAPTURE_HPP

#include "Utility/Capture.hpp"

namespace Nebulite::Constants {
/**
 * @struct Nebulite::Constants::StandardCapture
 * @brief Standard capture outputs for various warnings, errors, etc.
 * @details The advantage of these standard errors is the reusability in multiple places.
 *
 */
struct StandardCapture {

    struct Warning {
        struct SDL {
            static void generic(Utility::Capture& cap) {
                cap.warning.println("SDL encountered a non-critical issue.");
            }
        };

        struct Renderer {
            static void snapshotFailed(Utility::Capture& cap) {
                cap.warning.println("Renderer snapshot failed.");
            }
        };

        struct Functional {
            static void functionCallInvalid(Utility::Capture& cap) {
                cap.warning.println("Requested function call is invalid.");
            }

            static void invalidArgcArgvParsing(Utility::Capture& cap) {
                cap.warning.println("argc/argv parsing error.");
            }

            static void tooManyArgs(Utility::Capture& cap) {
                cap.warning.println("Too Many Arguments in function call");
            }

            static void tooFewArgs(Utility::Capture& cap) {
                cap.warning.println("Too Few Arguments in function call");
            }

            static void unknownArg(Utility::Capture& cap) {
                cap.warning.println("Unknown Argument Error");
            }

            static void featureNotImplemented(Utility::Capture& cap) {
                cap.warning.println("Requested feature of functioncall is not implemented");
            }

            static void bindingCollision(Utility::Capture& cap) {
                cap.warning.println("Binding Collision: Failed to bind a function, category or variable.");
            }

            static void bindingCollisionExpected(Utility::Capture& cap) {
                cap.warning.println("Binding Collision Expected: No collision occurred when one was expected.");
            }
        };
    };

    struct Error {
        struct SDL {
            static void initFailed(Utility::Capture& cap) {
                cap.error.println("SDL Renderer could not be initialized. This may cause rendering issues.");
            }
            static void settingTargetFailed(Utility::Capture& cap) {
                cap.error.println("SDL Renderer target could not be set.");
            }
        };

        struct Renderer {
            static void notInitialized(Utility::Capture& cap) {
                cap.error.println("Renderer not initialized.");
            }
        };

        struct Texture {
            static void notFound(Utility::Capture& cap) {
                cap.error.println("Texture not found.");
            }

            static void copyFailed(Utility::Capture& cap) {
                cap.error.println("Texture copy failed.");
            }

            static void colorUnsupported(Utility::Capture& cap) {
                cap.error.println("Texture color format unsupported.");
            }

            static void lockFailed(Utility::Capture& cap) {
                cap.error.println("Texture lock failed.");
            }

            static void queryFailed(Utility::Capture& cap) {
                cap.error.println("Texture query failed.");
            }

            static void modificationFailed(Utility::Capture& cap) {
                cap.error.println("Texture modification failed.");
            }

            static void invalid(Utility::Capture& cap) {
                cap.error.println("Texture is invalid.");
            }
        };

        struct Audio {
            static void deviceInitFailed(Utility::Capture& cap) {
                cap.error.println("Audio device could not be initialized.");
            }
        };

        struct Functional {
            static void functionNotImplemented(Utility::Capture& cap) {
                cap.error.println("Requested function not implemented.");
            }
        };

        struct File {
            static void invalidFile(Utility::Capture& cap) {
                cap.error.println("Requested file is invalid.");
            }
        };

        struct Ruleset {
            static void parsingFailed(Utility::Capture& cap) {
                cap.error.println("Ruleset parsing failed.");
            }
        };
    };
};
} // namespace Nebulite::Constants
#endif // NEBULITE_CONSTANTS_STANDARD_CAPTURE_HPP
