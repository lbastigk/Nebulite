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

// NOLINTNEXTLINE
bool Assertions::assertTypeObject(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(valueKey) != Data::KeyType::object) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is not an object";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeArray(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(valueKey) != Data::KeyType::array) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is not an array";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeBasicValue(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(valueKey) != Data::KeyType::value) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is not a basic value";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

void Assertions::printUserDefinedMessage(std::span<std::string const> const& args){
    if (args.size() < 2) {
        return; // No message provided
    }
    Error::println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

} // namespace Nebulite::TransformationModule
