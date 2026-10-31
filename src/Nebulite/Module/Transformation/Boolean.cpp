//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Transformation/Boolean.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Boolean::bindTransformations() {
    bindTransformation(&Boolean::booleanNot, booleanNotName, booleanNotDesc);
    bindTransformation(&Boolean::booleanToSigned, booleanToSignedName, booleanToSignedDesc);
    bindTransformation(&Boolean::booleanAnd, booleanAndName, booleanAndDesc);
    bindTransformation(&Boolean::booleanOr, booleanOrName, booleanOrDesc);
    bindTransformation(&Boolean::booleanXor, booleanXorName, booleanXorDesc);
    bindTransformation(&Boolean::booleanNand, booleanNandName, booleanNandDesc);
    bindTransformation(&Boolean::booleanNor, booleanNorName, booleanNorDesc);
    bindTransformation(&Boolean::booleanXnor, booleanXnorName, booleanXnorDesc);
}

bool Boolean::booleanNot(Data::JsonScope& jsonDoc){
    jsonDoc.set<bool>(rootKey, !jsonDoc.get<bool>(rootKey).value_or(false));
    return true;
}

bool Boolean::booleanToSigned(Data::JsonScope& jsonDoc){
    auto const val = jsonDoc.get<bool>(rootKey).value_or(false);
    jsonDoc.set<int>(rootKey, val ? 1 : -1);
    return true;
}

bool Boolean::booleanAnd(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    bool result = true;
    for (auto const item : jsonDoc.arrayItemsAs<bool>(rootKey)) {
        result = result && item.value_or(false);
    }
    jsonDoc.set<bool>(rootKey, result);
    return true;
}

bool Boolean::booleanOr(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    bool result = false;
    for (auto const item : jsonDoc.arrayItemsAs<bool>(rootKey)) {
        result = result || item.value_or(false);
    }
    jsonDoc.set<bool>(rootKey, result);
    return true;
}

bool Boolean::booleanXor(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    bool result = false;
    for (auto const item : jsonDoc.arrayItemsAs<bool>(rootKey)) {
        result = result != item.value_or(false);
    }
    jsonDoc.set<bool>(rootKey, result);
    return true;
}

bool Boolean::booleanNand(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    bool result = true;
    for (auto const item : jsonDoc.arrayItemsAs<bool>(rootKey)) {
        result = result && item.value_or(false);
    }
    jsonDoc.set<bool>(rootKey, !result);
    return true;
}

bool Boolean::booleanNor(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    bool result = false;
    for (auto const item : jsonDoc.arrayItemsAs<bool>(rootKey)) {
        result = result || item.value_or(false);
    }
    jsonDoc.set<bool>(rootKey, !result);
    return true;
}

bool Boolean::booleanXnor(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }
    bool result = true;
    for (auto const item : jsonDoc.arrayItemsAs<bool>(rootKey)) {
        result = result != item.value_or(false);
    }
    jsonDoc.set<bool>(rootKey, result);
    return true;
}

} // namespace Nebulite::Module::Transformation
