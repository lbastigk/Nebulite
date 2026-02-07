//------------------------------------------
// Includes

// Nebulite
#include "Core/JsonScope.hpp"
#include "TransformationModule/General.hpp"

//------------------------------------------
namespace Nebulite::TransformationModule {

void General::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&General::setString, setStringName, setStringDesc);
    BIND_TRANSFORMATION_STATIC(&General::setInt, setIntName, setIntDesc);
    BIND_TRANSFORMATION_STATIC(&General::setDouble, setDoubleName, setDoubleDesc);
    BIND_TRANSFORMATION_STATIC(&General::setBool, setBoolName, setBoolDesc);
    BIND_TRANSFORMATION_STATIC(&General::removeMember, removeMemberName, removeMemberDesc);
    BIND_TRANSFORMATION_MEMBER(&General::setFromResult, setFromResultName, setFromResultDesc);
}

bool General::setString(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey + std::string(args[1]);
    auto const value = std::string(args[2]);
    jsonDoc->set<std::string>(key, value);
    return true;
}

bool General::setInt(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey + std::string(args[1]);
    try {
        int const value = std::stoi(args[2]);
        jsonDoc->set<int>(key, value);
        return true;
    } catch (...) {
        return false;
    }
}

bool General::setDouble(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey + std::string(args[1]);
    try {
        double const value = std::stod(args[2]);
        jsonDoc->set<double>(key, value);
        return true;
    } catch (...) {
        return false;
    }
}

bool General::setBool(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey + std::string(args[1]);
    std::string const valStr = args[2];
    bool const value = valStr == "true";
    jsonDoc->set<bool>(key, value);
    return true;
}

bool General::removeMember(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) return false;
    for (auto const& arg : args.subspan(1)) {
        auto const key = rootKey + std::string(arg);
        jsonDoc->removeMember(key);
    }
    return true;
}

bool General::setFromResult(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) const {
    if (args.size() < 2) return false;
    auto const key = rootKey + std::string(args[1]);
    auto const transformation = extractPotentiallyWrappedString(args.subspan(2));

    auto targs = Data::JSON::splitKeyWithTransformations(transformation);
    if (targs.empty()) {
        return false;
    }
    std::string const scopeKey = targs.front();
    targs.erase(targs.begin());

    Data::JSON transformationResult = jsonDoc->getSubDoc(rootKey + scopeKey);
    auto& scope = transformationResult.shareManagedScope("");
    static std::string const funcName = __FUNCTION__;
    for (auto const& targ : targs) {
        if (!transformationFuncTree->parseStr(funcName + " " + targ, &scope)) {
            return false; // Transformation failed
        }
    }
    jsonDoc->setSubDoc(key, transformationResult);
    return true;
}


} // namespace Nebulite::TransformationModule
