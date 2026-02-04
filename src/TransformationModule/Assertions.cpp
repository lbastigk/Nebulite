#include "TransformationModule/Assertions.hpp"

#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Assertions::bindTransformations() {
    transformationFuncTree->bindCategory(assertName, assertDesc);
    transformationFuncTree->bindCategory(assertTypeName, assertTypeDesc);
    BIND_TRANSFORMATION_STATIC(&Assertions::assertNonEmpty, assertNonEmptyName, assertNonEmptyDesc);
    BIND_TRANSFORMATION_STATIC(&Assertions::assertTypeObject, assertTypeObjectName, assertTypeObjectDesc);
    BIND_TRANSFORMATION_STATIC(&Assertions::assertTypeArray, assertTypeArrayName, assertTypeArrayDesc);
    BIND_TRANSFORMATION_STATIC(&Assertions::assertTypeBasicValue, assertTypeBasicValueName, assertTypeBasicValueDesc);
}

void Assertions::printUserDefinedMessage(std::span<std::string const> const& args){
    if (args.size() < 2) {
        return; // No message provided
    }
    Error::println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

// NOLINTNEXTLINE
bool Assertions::assertNonEmpty(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertNonEmptyName) + ": JSON value is null";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeObject(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::object) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertTypeObjectName) + ": JSON value is not an object";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeArray(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertTypeArrayName) + ": JSON value is not an array";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeBasicValue(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::value) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertTypeBasicValueName) + ": JSON value is not a basic value";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

} // namespace Nebulite::TransformationModule
