#ifndef CONSTANTS_STANDARDCAPTURE_HPP
#define CONSTANTS_STANDARDCAPTURE_HPP

//------------------------------------------
// Includes

// Standard Library
#include "Constants/Event.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Constants {
/**
 * @struct Nebulite::Constants::StandardCapture
 * @brief Standard capture outputs for various warnings, errors, etc.
 * @details The advantage of these standard errors is the reusability in multiple places.
 *          Takes care of both the logging and the proper Event return value.
 */
struct StandardCapture {
    struct Warning {
        struct Renderer {
            static Event snapshotFailed(Utility::IO::Capture& cap);
        };

        struct Functional {
            static Event functionCallInvalid(Utility::IO::Capture& cap);

            static Event invalidArgcArgvParsing(Utility::IO::Capture& cap);

            static Event tooManyArgs(Utility::IO::Capture& cap);

            static Event tooFewArgs(Utility::IO::Capture& cap);

            static Event unknownArg(Utility::IO::Capture& cap);

            static Event featureNotImplemented(Utility::IO::Capture& cap);

            static Event bindingCollision(Utility::IO::Capture& cap);

            static Event bindingCollisionExpected(Utility::IO::Capture& cap);
        };
    };

    struct Error {
        struct SDL {
            static Event initFailed(Utility::IO::Capture& cap);

            static Event settingTargetFailed(Utility::IO::Capture& cap);
        };

        struct Renderer {
            static Event notInitialized(Utility::IO::Capture& cap);
        };

        struct Texture {
            static Event notFound(Utility::IO::Capture& cap);

            static Event copyFailed(Utility::IO::Capture& cap);

            static Event colorUnsupported(Utility::IO::Capture& cap);

            static Event lockFailed(Utility::IO::Capture& cap);

            static Event queryFailed(Utility::IO::Capture& cap);

            static Event modificationFailed(Utility::IO::Capture& cap);

            static Event invalid(Utility::IO::Capture& cap);
        };

        struct Audio {
            static Event deviceInitFailed(Utility::IO::Capture& cap);
        };

        struct Functional {
            static Event functionNotImplemented(Utility::IO::Capture& cap);
        };

        struct File {
            static Event invalidFile(Utility::IO::Capture& cap);

            static Event couldNotWriteFile(Utility::IO::Capture& cap);
        };

        struct Ruleset {
            static Event parsingFailed(Utility::IO::Capture& cap);
        };
    };
};
} // namespace Nebulite::Constants
#endif // CONSTANTS_STANDARDCAPTURE_HPP
