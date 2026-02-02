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
    if (args.size() != 2) return false;
    auto const key = rootKey + std::string(args[0]);
    auto const value = std::string(args[1]);
    jsonDoc->set<std::string>(key, value);
    return true;
}

bool General::setInt(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) return false;
    auto const key = rootKey + std::string(args[0]);
    try {
        int const value = std::stoi(args[1]);
        jsonDoc->set<int>(key, value);
        return true;
    } catch (...) {
        return false;
    }
}

bool General::setDouble(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) return false;
    auto const key = rootKey + std::string(args[0]);
    try {
        double const value = std::stod(args[1]);
        jsonDoc->set<double>(key, value);
        return true;
    } catch (...) {
        return false;
    }
}

bool General::setBool(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) return false;
    auto const key = rootKey + std::string(args[0]);
    std::string const valStr = args[1];
    bool const value = valStr == "true";
    jsonDoc->set<bool>(key, value);
    return true;
}

bool General::removeMember(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.empty()) return false;
    for (auto const& arg : args) {
        auto const key = rootKey + std::string(arg);
        jsonDoc->removeMember(key);
    }
    return true;
}

bool General::setFromResult(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) const {
    if (args.size() < 2) return false;
    auto const key = rootKey + std::string(args[0]);
    auto const transformation = Utility::StringHandler::recombineArgs(args.subspan(1));

    Data::JSON transformationResult; // Placeholder for the result of the transformation execution.
    auto& scope = transformationResult.shareManagedScope("");
    transformationFuncTree->parseStr(transformation, &scope);
    jsonDoc->setSubDoc(key, transformationResult);
    return true;
}


} // namespace Nebulite::TransformationModule
