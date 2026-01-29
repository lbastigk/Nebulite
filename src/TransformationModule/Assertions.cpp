#include "TransformationModule/Assertions.hpp"

#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Assertions::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Assertions::assertNonEmpty, assertNonEmptyName, assertNonEmptyDesc);
}

// NOLINTNEXTLINE
bool Assertions::assertNonEmpty(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(valueKey) == Data::KeyType::null) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is null";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

void Assertions::printUserDefinedMessage(std::span<std::string const> const& args){
    if (args.size() < 2) {
        return;
    }
    Nebulite::Error::println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

} // namespace Nebulite::TransformationModule
