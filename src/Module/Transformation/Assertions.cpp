#include "Module/Transformation/Assertions.hpp"

#include "Nebulite.hpp"

namespace Nebulite::Module::Transformation {

void Assertions::bindTransformations() {
    bindCategory(assertName, assertDesc);
    bindTransformation(&Assertions::assertNonEmpty, assertNonEmptyName, assertNonEmptyDesc);

    bindCategory(assertTypeName, assertTypeDesc);
    bindTransformation(&Assertions::assertTypeObject, assertTypeObjectName, assertTypeObjectDesc);
    bindTransformation(&Assertions::assertTypeArray, assertTypeArrayName, assertTypeArrayDesc);
    bindTransformation(&Assertions::assertTypeBasicValue, assertTypeBasicValueName, assertTypeBasicValueDesc);

    bindCategory(assertEqualsName, assertEqualsDesc);
    bindTransformation(&Assertions::assertEqualsString, assertEqualsStringName, assertEqualsStringDesc);
}

void Assertions::printUserDefinedMessage(std::span<std::string const> const& args){
    if (args.size() < 2) {
        return; // No message provided
    }
    Global::capture().error.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

// NOLINTNEXTLINE
bool Assertions::assertNonEmpty(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertNonEmptyName) + ": JSON value is null";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeObject(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::object) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertTypeObjectName) + ": JSON value is not an object";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeArray(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertTypeArrayName) + ": JSON value is not an array";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertTypeBasicValue(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::value) {
        printUserDefinedMessage(args);
        static std::string errorMessage = std::string(assertTypeBasicValueName) + ": JSON value is not a basic value";
        throw std::runtime_error(errorMessage);
    }
    return true;
}

// NOLINTNEXTLINE
bool Assertions::assertEqualsString(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    auto const expected = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (jsonDoc->memberType(rootKey) != Data::KeyType::value) {
        throw std::runtime_error(std::string(assertEqualsStringName) + ": Current JSON value is not a basic value, expected string: " + expected);
    }
    if (auto const actual = jsonDoc->get<std::string>(rootKey).value_or("null"); actual != expected) {
        throw std::runtime_error(std::string(assertEqualsStringName) + ": JSON value '" + actual + "' does not equal expected string '" + expected + "'");
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
