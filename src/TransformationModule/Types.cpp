#include "Data/Document/JsonScope.hpp"
#include "TransformationModule/Types.hpp"

namespace Nebulite::TransformationModule {

void Types::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Types::typeAsNumber, typeAsNumberName, typeAsNumberDesc);
    BIND_TRANSFORMATION_STATIC(&Types::typeAsString, typeAsStringName, typeAsStringDesc);
    BIND_TRANSFORMATION_STATIC(&Types::serialize, serializeName, serializeDesc);
    BIND_TRANSFORMATION_STATIC(&Types::deserialize, deserializeName, deserializeDesc);
}

bool Types::typeAsNumber(Data::JsonScope* jsonDoc) {
    jsonDoc->set<int>(rootKey, static_cast<int>(jsonDoc->memberType(rootKey)));
    return true;
}

bool Types::typeAsString(Data::JsonScope* jsonDoc) {
    std::string const type = jsonDoc->memberTypeString(rootKey);
    jsonDoc->set<std::string>(rootKey, type);
    return true;
}

bool Types::serialize(Data::JsonScope* jsonDoc) {
    std::string const serialized = jsonDoc->serialize();
    jsonDoc->set<std::string>(rootKey, serialized);
    return true;
}

bool Types::deserialize(Data::JsonScope* jsonDoc) {
    auto const serialized = jsonDoc->get<std::string>(rootKey).value_or("");
    Data::JSON tempDoc;
    if (!Data::JSON::isJsonOrJsonc(serialized)) {
        return false;
    }
    tempDoc.deserialize(serialized);
    jsonDoc->setSubDoc(rootKey, tempDoc);
    return true;
}

bool Types::exists(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() > 2) {
        return false;
    }
    std::string const key = args.size() == 2 ? args[1] : "";
    if (jsonDoc->memberType(rootKey + key) == Data::KeyType::null) {
        jsonDoc->set<bool>(rootKey, false);
    }
    else {
        jsonDoc->set<bool>(rootKey, true);
    }
    return true;
}

} // namespace Nebulite::TransformationModule
