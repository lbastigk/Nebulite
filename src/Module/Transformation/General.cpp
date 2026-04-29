//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Transformation/General.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void General::bindTransformations() {
    bindTransformation(&General::setString, setStringName, setStringDesc);
    bindTransformation(&General::setInt, setIntName, setIntDesc);
    bindTransformation(&General::setDouble, setDoubleName, setDoubleDesc);
    bindTransformation(&General::setBool, setBoolName, setBoolDesc);
    bindTransformation(&General::removeMember, removeMemberName, removeMemberDesc);
    bindTransformation(&General::setFromResult, setFromResultName, setFromResultDesc);
    bindTransformation(&General::assign, assignName, assignDesc);
    bindTransformation(&General::asString, asStringName, asStringDesc);
    bindTransformation(&General::formatNumber, formatNumberName, formatNumberDesc);

}

bool General::setString(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey + std::string(args[1]);
    auto const value = std::string(args[2]);
    jsonDoc->set<std::string>(key, value);
    return true;
}

bool General::setInt(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
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

bool General::setDouble(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
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

bool General::setBool(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey + std::string(args[1]);
    std::string const& valStr = args[2];
    bool const value = valStr == "true";
    jsonDoc->set<bool>(key, value);
    return true;
}

bool General::removeMember(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) return false;
    for (auto const& arg : args.subspan(1)) {
        auto const key = rootKey + std::string(arg);
        jsonDoc->removeMember(key);
    }
    return true;
}

bool General::setFromResult(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) return false;
    auto const key = rootKey + std::string(args[1]);
    auto const innerEval = Utility::StringHandler::recombineArgs(args.subspan(2));

    Interaction::ContextScope const context{*jsonDoc, *jsonDoc, *jsonDoc}; // Using the same scope for self, other and global since we only have access to one scope here
    Data::JSON const transformationResult = Interaction::Logic::Expression::evalAsJson(innerEval, context); // Evaluating the expression to get the transformation result
    jsonDoc->setSubDoc(key, transformationResult);
    return true;
}

bool General::assign(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 1) return false;
    Interaction::Logic::Assignment ass;
    ass.parse(Utility::StringHandler::recombineArgs(args.subspan(1)));
    Interaction::ContextScope const context{*jsonDoc, *jsonDoc, *jsonDoc};
    ass.apply(context);
    return true;
}

bool General::asString(Data::JsonScope* jsonDoc){
    switch (jsonDoc->memberType(rootKey)) {
        case Data::KeyType::null:
            jsonDoc->set(rootKey, "null");
            break;
        case Data::KeyType::value:
            // Nothing to do
            break;
        case Data::KeyType::array:
            jsonDoc->set(rootKey, "[array]");
            break;
        case Data::KeyType::object:
            jsonDoc->set(rootKey, "{object}");
            break;
        default:
            std::unreachable();
    }
    return true;
}

bool General::formatNumber(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) != Data::KeyType::value) return false;
    if (args.size() != 2) return false;
    auto const value = jsonDoc->get<std::string>(rootKey);
    if (!value.has_value()) return false;

    if (Utility::StringHandler::isNumber(value.value())) {
        auto const fmt = Interaction::Logic::Expression::Formatter::readFormatter(args[1]);
        jsonDoc->set(rootKey, fmt.format(std::stod(value.value())));
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
