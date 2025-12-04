#include "Nebulite.hpp"


namespace Nebulite {

Core::GlobalSpace& global() {
    static Core::GlobalSpace globalSpaceInstance("Nebulite");
    return globalSpaceInstance;
}

Nebulite::Utility::CaptureStream& cout() {
    return Nebulite::Utility::Capture::cout();
}

Nebulite::Utility::CaptureStream& cerr() {
    return Nebulite::Utility::Capture::cerr();
}

} // namespace Nebulite