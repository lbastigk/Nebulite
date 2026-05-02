//------------------------------------------
// Includes

// Standard library
#include <regex>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/Assertions.hpp"
#include "Module/Transformation/Requirements.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Requirements::bindTransformations() {
    bindCategory(requireName, requireDesc);
    bindTransformation(&Requirements::requireNonEmpty, requireNonEmptyName, requireNonEmptyDesc);

    bindCategory(requireTypeName, requireTypeDesc);
    bindTransformation(&Requirements::requireTypeObject, requireTypeObjectName, requireTypeObjectDesc);
    bindTransformation(&Requirements::requireTypeArray, requireTypeArrayName, requireTypeArrayDesc);
    bindTransformation(&Requirements::requireTypeBasicValue, requireTypeBasicValueName, requireTypeBasicValueDesc);
}

void Requirements::printUserDefinedMessage(std::span<std::string const> const& args){
    if (args.size() < 2) {
        return; // No message provided
    }
    Global::capture().error.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

bool Requirements::requireNonEmpty(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc) {
    try {
        Assertions::assertNonEmpty(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireEmpty(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc){
    try {
        Assertions::assertEmpty(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeObject(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc) {
    try {
        Assertions::assertTypeObject(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeArray(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc) {
    try {
        Assertions::assertTypeArray(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeBasicValue(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc) {
    try {
        Assertions::assertTypeBasicValue(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireMatchRegex(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc){
    try {
        Assertions::assertMatchRegex(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireEqualsString(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc) {
    try {
        Assertions::assertEqualsString(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireEqualsInt(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc){
    try {
        Assertions::assertEqualsInt(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
