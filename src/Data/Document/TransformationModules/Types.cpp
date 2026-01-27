#include "Data/Document/TransformationModules/Types.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Data::TransformationModules {

void Types::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Types::typeAsNumber, typeAsNumberName, typeAsNumberDesc);
    BIND_TRANSFORMATION_STATIC(&Types::typeAsString, typeAsStringName, typeAsStringDesc);
    BIND_TRANSFORMATION_STATIC(&Types::serialize, serializeName, serializeDesc);
    BIND_TRANSFORMATION_STATIC(&Types::deserialize, deserializeName, deserializeDesc);
}

bool Types::typeAsNumber(Core::JsonScope* jsonDoc) {
    jsonDoc->set<int>(valueKey, static_cast<int>(jsonDoc->memberType(valueKey)));
    return true;
}

bool Types::typeAsString(Core::JsonScope* jsonDoc) {
    // TODO: Add a getTypeAsString function to JSON class to avoid code duplication
    //       - array -> "array::size"
    //       - object -> "object::size"
    //       - null -> "null"
    //       - string -> "value:string:size"
    //       - int -> "value:int:bitwidth"
    //       - double -> "value:double:bitwidth"
    //       - bool -> "value:bool"
    //       etc...
    //       <baseType>:<subType>:<sizeInfo>
    //       e.g.: "value:int:32" or "value:string:10"
    //       Perhaps with additional arg to control the format?
    switch (jsonDoc->memberType(valueKey)) {
    case KeyType::value: {
        // General type is "value", but we can be more specific by using getVariant or even better:
        // TODO: see above comment
        jsonDoc->set<std::string>(valueKey, "value");
    }
        break;
    case KeyType::array:
        jsonDoc->set<std::string>(valueKey, "array");
        break;
    case KeyType::object:
        jsonDoc->set<std::string>(valueKey, "object");
        break;
    case KeyType::null:
    default:
        jsonDoc->set<std::string>(valueKey, "null");
        break;
    }
    return true;
}

bool Types::serialize(Core::JsonScope* jsonDoc) {
    std::string const serialized = jsonDoc->serialize();
    jsonDoc->set<std::string>(valueKey, serialized);
    return true;
}

bool Types::deserialize(Core::JsonScope* jsonDoc) {
    auto const serialized = jsonDoc->get<std::string>(valueKey, "");
    JSON tempDoc;
    if (!JSON::isJsonOrJsonc(serialized)) {
        return false;
    }
    tempDoc.deserialize(serialized);
    jsonDoc->setSubDoc(valueKey, tempDoc);
    return true;
}

} // namespace Nebulite::Data::TransformationModules
