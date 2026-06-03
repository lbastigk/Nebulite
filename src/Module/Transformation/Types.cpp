//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>
#include <utility>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/KeyType.hpp"
#include "Module/Transformation/Types.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Types::bindTransformations() {
    bindTransformation(&Types::defaultToString, defaultToStringName, defaultToStringDesc);
    bindTransformation(&Types::typeAsNumber, typeAsNumberName, typeAsNumberDesc);
    bindTransformation(&Types::typeAsSimpleString, typeAsSimpleStringName, typeAsSimpleStringDesc);
    bindTransformation(&Types::typeAsString, typeAsStringName, typeAsStringDesc);
    bindTransformation(&Types::serialize, serializeName, serializeDesc);
    bindTransformation(&Types::deserialize, deserializeName, deserializeDesc);
    bindTransformation(&Types::exists, existsName, existsDesc);
}

bool Types::defaultToString(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) == Data::KeyType::null) {
        jsonDoc->set<std::string>(rootKey, Utility::StringHandler::recombineArgs(args.subspan(1)));
    }
    return true;
}

bool Types::typeAsNumber(Data::JsonScope* jsonDoc) {
    jsonDoc->set<int>(rootKey, static_cast<int>(jsonDoc->memberType(rootKey)));
    return true;
}

bool Types::typeAsSimpleString(Data::JsonScope* jsonDoc){
    switch (jsonDoc->memberType(rootKey)) {
        case Data::KeyType::null:
            jsonDoc->set<std::string>(rootKey, "null");
            break;
        case Data::KeyType::value:
            jsonDoc->set<std::string>(rootKey, "value");
            break;
        case Data::KeyType::array:
            jsonDoc->set<std::string>(rootKey, "array");
            break;
        case Data::KeyType::object:
            jsonDoc->set<std::string>(rootKey, "object");
            break;
        default:
            std::unreachable();
    }
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
    if (!Data::JSON::isJsonOrJsonc(serialized)) {
        return false;
    }
    Data::JSON tempDoc;
    tempDoc.deserialize(serialized);
    jsonDoc->setSubDoc(rootKey, tempDoc);
    return true;
}

bool Types::exists(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() > 2) {
        return false;
    }
    if (jsonDoc->memberType(rootKey.addMember(args.size() == 2 ? args[1] : "")) == Data::KeyType::null) {
        jsonDoc->set<bool>(rootKey, false);
    }
    else {
        jsonDoc->set<bool>(rootKey, true);
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
