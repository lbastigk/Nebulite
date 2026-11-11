//------------------------------------------
// Includes

// Standard library
#include <stdexcept>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Transformation/Assertions.hpp"
#include "Nebulite/Module/Transformation/Requirements.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

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
    bindTransformation(&Requirements::requireTypeNumeric, requireTypeNumericName, requireTypeNumericDesc);
    bindTransformation(&Requirements::requireTypeNumericOrNumericString, requireTypeNumericOrNumericStringName, requireTypeNumericOrNumericStringDesc);

    bindCategory(requireMatchName, requireMatchDesc);
    bindTransformation(&Requirements::requireMatchRegex, requireMatchRegexName, requireMatchRegexDesc);

    bindCategory(requireEqualsName, requireEqualsDesc);
    bindTransformation(&Requirements::requireEqualsString, requireEqualsStringName, requireEqualsStringDesc);
    bindTransformation(&Requirements::requireEqualsInt, requireEqualsIntName, requireEqualsIntDesc);
}

bool Requirements::requireTrue(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertTrue(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireFalse(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertFalse(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireNonEmpty(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertNonEmpty(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireEmpty(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertEmpty(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeObject(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertTypeObject(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeArray(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertTypeArray(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeBasicValue(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertTypeBasicValue(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertTypeNumeric(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireTypeNumericOrNumericString(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertTypeNumericOrNumericString(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireMatchRegex(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    if (args.size() < 2) return false;
    try {
        Assertions::assertMatchRegex(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireEqualsString(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc) {
    try {
        Assertions::assertEqualsString(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

bool Requirements::requireEqualsInt(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc){
    try {
        Assertions::assertEqualsInt(args, jsonDoc);
    }
    catch (std::runtime_error const&) {
        return false;
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
