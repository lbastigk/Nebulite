//------------------------------------------
// Includes

// Standard library
#include <span>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Transformation/Assertions.hpp"
#include "Nebulite/Module/Transformation/Requirements.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Requirements::bindTransformations() {
    bindCategory(requireName, requireDesc);
    bindTransformation(&Requirements::requireTrue, requireTrueName, requireTrueDesc);
    bindTransformation(&Requirements::requireFalse, requireFalseName, requireFalseDesc);
    bindTransformation(&Requirements::requireEmpty, requireEmptyName, requireEmptyDesc);
    bindTransformation(&Requirements::requireNonEmpty, requireNonEmptyName, requireNonEmptyDesc);

    bindCategory(requireTypeName, requireTypeDesc);
    bindTransformation(&Requirements::requireTypeObject, requireTypeObjectName, requireTypeObjectDesc);
    bindTransformation(&Requirements::requireTypeArray, requireTypeArrayName, requireTypeArrayDesc);
    bindTransformation(&Requirements::requireTypeBasicValue, requireTypeBasicValueName, requireTypeBasicValueDesc);

    bindCategory(requireMatchName, requireMatchDesc);
    bindTransformation(&Requirements::requireMatchRegex, requireMatchRegexName, requireMatchRegexDesc);

    bindCategory(requireEqualsName, requireEqualsDesc);
    bindTransformation(&Requirements::requireEqualsString, requireEqualsStringName, requireEqualsStringDesc);
    bindTransformation(&Requirements::requireEqualsInt, requireEqualsIntName, requireEqualsIntDesc);
}

void Requirements::printUserDefinedMessage(std::span<std::string_view const> const& args){
    if (args.size() < 2) {
        return; // No message provided
    }
    Global::capture().error.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

bool Requirements::requireTrue(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertTrue(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireFalse(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertFalse(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireNonEmpty(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertNonEmpty(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireEmpty(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertEmpty(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeObject(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertTypeObject(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeArray(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertTypeArray(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeBasicValue(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertTypeBasicValue(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireMatchRegex(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertMatchRegex(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireEqualsString(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertEqualsString(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool Requirements::requireEqualsInt(std::span<std::string_view const> const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertEqualsInt(args, jsonDoc);
    }
    catch (...) {
        return false;
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
