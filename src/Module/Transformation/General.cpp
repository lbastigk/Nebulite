//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Transformation/General.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void General::bindTransformations() {
    bindTransformation(&General::setString, setStringName, setStringDesc);
    bindTransformation(&General::setInt, setIntName, setIntDesc);
    bindTransformation(&General::setDouble, setDoubleName, setDoubleDesc);
    bindTransformation(&General::setBool, setBoolName, setBoolDesc);
    bindTransformation(&General::removeMember, removeMemberName, removeMemberDesc);
    bindTransformation(&General::assign, assignName, assignDesc);
}

bool General::setString(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey.addMember(args[1]);
    auto const value = std::string(args[2]);
    jsonDoc->set<std::string>(key, value);
    return true;
}

bool General::setInt(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey.addMember(args[1]);
    try {
        int const value = std::stoi(std::string(args[2]));
        jsonDoc->set<int>(key, value);
        return true;
    } catch (...) {
        return false;
    }
}

bool General::setDouble(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey.addMember(args[1]);
    try {
        double const value = std::stod(std::string(args[2]));
        jsonDoc->set<double>(key, value);
        return true;
    } catch (...) {
        return false;
    }
}

bool General::setBool(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3) return false;
    auto const key = rootKey.addMember(args[1]);
    auto const& valStr = args[2];
    bool const value = valStr == "true";
    jsonDoc->set<bool>(key, value);
    return true;
}

bool General::removeMember(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) return false;
    for (auto const& arg : args.subspan(1)) {
        auto const key = rootKey.addMember(arg);
        jsonDoc->removeMember(key);
    }
    return true;
}

bool General::assign(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.empty()) return false;
    Interaction::Logic::Assignment ass;
    ass.parse(Utility::StringHandler::recombineArgs(args.subspan(1)));
    Interaction::ContextScope const context{*jsonDoc, *jsonDoc, *jsonDoc};
    ass.apply(context);
    return true;
}

} // namespace Nebulite::Module::Transformation
