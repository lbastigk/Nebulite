#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "TransformationModule/Requirements.hpp"

namespace Nebulite::TransformationModule {

void Requirements::bindTransformations() {
    transformationFuncTree->bindCategory(requireName, requireDesc);
    transformationFuncTree->bindCategory(requireTypeName, requireTypeDesc);
    BIND_TRANSFORMATION_STATIC(&Requirements::requireNonEmpty, requireNonEmptyName, requireNonEmptyDesc);
    BIND_TRANSFORMATION_STATIC(&Requirements::requireTypeObject, requireTypeObjectName, requireTypeObjectDesc);
    BIND_TRANSFORMATION_STATIC(&Requirements::requireTypeArray, requireTypeArrayName, requireTypeArrayDesc);
    BIND_TRANSFORMATION_STATIC(&Requirements::requireTypeBasicValue, requireTypeBasicValueName, requireTypeBasicValueDesc);
}

void Requirements::printUserDefinedMessage(std::span<std::string const> const& args){
    if (args.size() < 2) {
        return; // No message provided
    }
    Error::println(Utility::StringHandler::recombineArgs(args.subspan(1)));
}

// NOLINTNEXTLINE
bool Requirements::requireNonEmpty(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        printUserDefinedMessage(args);
        return false;
    }
    return true;
}

// NOLINTNEXTLINE
bool Requirements::requireTypeObject(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::object) {
        printUserDefinedMessage(args);
        return false;
    }
    return true;
}

// NOLINTNEXTLINE
bool Requirements::requireTypeArray(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) {
        printUserDefinedMessage(args);
        return false;
    }
    return true;
}

// NOLINTNEXTLINE
bool Requirements::requireTypeBasicValue(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (jsonDoc->memberType(rootKey) != Data::KeyType::value) {
        printUserDefinedMessage(args);
        return false;
    }
    return true;
}

} // namespace Nebulite::TransformationModule
