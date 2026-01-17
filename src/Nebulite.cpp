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

Utility::CaptureStream& cout() {
    return Utility::Capture::cout();
}

Utility::CaptureStream& cerr() {
    return Utility::Capture::cerr();
}

} // namespace Nebulite
