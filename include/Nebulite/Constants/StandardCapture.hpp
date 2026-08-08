#ifndef NEBULITE_CONSTANTS_STANDARDCAPTURE_HPP
#define NEBULITE_CONSTANTS_STANDARDCAPTURE_HPP

//------------------------------------------
// Includes

// Standard Library
#include "Nebulite/Constants/Event.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Utility::Io {
class Capture;
} // namespace Nebulite::Utility::Io

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
            static Event snapshotFailed(Utility::Io::Capture& cap);
        };

        struct Functional {
            static Event functionCallInvalid(Utility::Io::Capture& cap);

            static Event invalidArgcArgvParsing(Utility::Io::Capture& cap);

            static Event tooManyArgs(Utility::Io::Capture& cap);

            static Event tooFewArgs(Utility::Io::Capture& cap);

            static Event unknownArg(Utility::Io::Capture& cap);

            static Event invalidArgument(Utility::Io::Capture& cap);

            static Event featureNotImplemented(Utility::Io::Capture& cap);

            static Event bindingCollision(Utility::Io::Capture& cap);

            static Event bindingCollisionExpected(Utility::Io::Capture& cap);
        };
    };

    struct Error {
        struct Sdl {
            static Event initFailed(Utility::Io::Capture& cap);

            static Event settingTargetFailed(Utility::Io::Capture& cap);
        };

        struct Renderer {
            static Event notInitialized(Utility::Io::Capture& cap);
        };

        struct Texture {
            static Event notFound(Utility::Io::Capture& cap);

            static Event copyFailed(Utility::Io::Capture& cap);

            static Event colorUnsupported(Utility::Io::Capture& cap);

            static Event lockFailed(Utility::Io::Capture& cap);

            static Event queryFailed(Utility::Io::Capture& cap);

            static Event modificationFailed(Utility::Io::Capture& cap);

            static Event invalid(Utility::Io::Capture& cap);
        };

        struct Audio {
            static Event deviceInitFailed(Utility::Io::Capture& cap);
        };

        struct Functional {
            static Event functionNotImplemented(Utility::Io::Capture& cap);
        };

        struct File {
            static Event invalidFile(Utility::Io::Capture& cap);

            static Event couldNotWriteFile(Utility::Io::Capture& cap);
        };

        struct Ruleset {
            static Event parsingFailed(Utility::Io::Capture& cap);
        };
    };
};
} // namespace Nebulite::Constants
#endif // NEBULITE_CONSTANTS_STANDARDCAPTURE_HPP
