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
        Data::JSON element = jsonDoc->getSubDoc(elementKey);
        Core::JsonScope tempDoc;
        tempDoc.setSubDoc(valueKey, element);

        // Parse transformation command
        if (!transformationFuncTree->parseStr(cmd, &tempDoc)) {
            tempDoc.removeKey(valueKey);
        }
        Data::JSON transformedElement = tempDoc.getSubDoc(valueKey);
        jsonDoc->setSubDoc(elementKey, transformedElement);
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

// TODO: doesnt work...
bool Collection::getMultiple(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    std::vector<std::unique_ptr<Data::JSON>> values;
    for (auto const& key : args.subspan(1)) {
        values.push_back(std::make_unique<Data::JSON>());
        auto subDoc = jsonDoc->getSubDoc(valueKey + key);
        values.back()->copyFrom(subDoc);
    }

    // Create result array
    for (size_t i = 0; i < values.size(); ++i) {
        auto const arrayKey = valueKey + "[" + std::to_string(i) + "]";
        jsonDoc->setSubDoc(arrayKey, *values[i]);
    }
    return true;
}

} // namespace Nebulite::TransformationModule
