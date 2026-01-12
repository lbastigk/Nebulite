#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Utility/Capture.hpp"

namespace Nebulite {

Core::GlobalSpace& global() {
    static Core::GlobalSpace globalSpaceInstance("Nebulite");
    return globalSpaceInstance;
}

GlobalDocAccessor& globalDoc() {
    static GlobalDocAccessor globalDocAccessorInstance;
    return globalDocAccessorInstance;
}

Nebulite::Utility::CaptureStream& cout() {
    return Nebulite::Utility::Capture::cout();
}

Nebulite::Utility::CaptureStream& cerr() {
    return Nebulite::Utility::Capture::cerr();
}

} // namespace Nebulite
