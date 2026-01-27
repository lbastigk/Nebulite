#include "Data/Document/TransformationModules/Debug.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Data::TransformationModules {

void Debug::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Debug::echo, echoName, echoDesc);
    BIND_TRANSFORMATION_STATIC(&Debug::print, printName, printDesc);
}

bool Debug::echo(std::span<std::string const> const& args) {
    // Echo args to cout
    for (size_t i = 1; i < args.size(); ++i) {
        Utility::Capture::cout() << args[i];
        if (i < args.size() - 1) {
            Utility::Capture::cout() << " ";
        }
    }
    Utility::Capture::cout() << Utility::Capture::endl;
    return true;
}

// NOLINTNEXTLINE
bool Debug::print(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    // Print to cout, no modifications
    if (args.size() > 1) {
        Utility::Capture::cout() << jsonDoc->serialize(valueKey + args[1]) << Utility::Capture::endl;
    } else {
        Utility::Capture::cout() << jsonDoc->serialize() << Utility::Capture::endl;
    }
    return true;
}

} // namespace Nebulite::Data::TransformationModules
