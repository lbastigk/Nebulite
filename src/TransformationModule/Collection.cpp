#include "TransformationModule/Collection.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Collection::bindTransformations() {
    // BIND_TRANSFORMATION_MEMBER(&Collection::filter, filterName, &filterDesc);
    BIND_TRANSFORMATION_MEMBER(&Collection::map, mapName, mapDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::get, getName, getDesc);
    BIND_TRANSFORMATION_STATIC(&Collection::getMultiple, getMultipleName, getMultipleDesc);
}

bool Collection::map(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) const {
    if (jsonDoc->memberType(valueKey) == Data::KeyType::value) {
        // Single value, we wrap it into an array
        // TODO: optimize by using moveMember, once available
        Data::JSON const tmp = jsonDoc->getSubDoc(valueKey);
        auto const key = valueKey + "[0]";
        jsonDoc->setSubDoc(key, tmp);
    }
    // Now we expect an array
    if (jsonDoc->memberType(valueKey) != Data::KeyType::array) {
        return false; // Not an array
    }
    // Re-join args into a single transformation command
    std::string cmd = __FUNCTION__;
    for (auto const& arg : args.subspan(1)) {
        cmd += " ";
        cmd += arg;
    }

    size_t const arraySize = jsonDoc->memberSize(valueKey);
    for (uint32_t idx = 0; idx < arraySize; ++idx) {
        // Set temp document with current element
        auto const elementKey = valueKey + "[" + std::to_string(idx) + "]";

        // Parse transformation command
        auto& scope = jsonDoc->shareScope(elementKey);
        if (!transformationFuncTree->parseStr(cmd, &scope)) {
            jsonDoc->removeKey(elementKey);
        }
    }
    return true;
}

bool Collection::get(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    auto const& key = valueKey + args[1];
    Data::JSON const subDoc = jsonDoc->getSubDoc(key);
    jsonDoc->setSubDoc(valueKey, subDoc);
    return true;
}

bool Collection::getMultiple(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    Data::JSON tmp;
    size_t i = 0;
    for (auto const& key : args.subspan(1)) {
        tmp.setSubDoc("[" + std::to_string(i) + "]", jsonDoc->getSubDoc(valueKey + key));
        ++i;
    }

    // Create result array
    jsonDoc->setSubDoc(valueKey, tmp);
    return true;
}

} // namespace Nebulite::TransformationModule
