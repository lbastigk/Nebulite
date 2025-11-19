//------------------------------------------
// Includes

#include "Utility/JSON.hpp"
#include "Utility/JsonModifier.hpp"

//------------------------------------------
namespace Nebulite::Utility {

JsonModifier::JsonModifier() {
    modifierFuncTree = std::make_unique<Interaction::Execution::FuncTree<bool, JSON*>>("JSON Modifier FuncTree", true, false);

    // Bind modifier functions
    bindModifierFunction(&JsonModifier::add, addName, &addDesc);
}

bool JsonModifier::parse(std::vector<std::string> const& args, JSON* jsonDoc){
    if(args.empty()){
            return false;
    }
    for (auto const& modifier : args) {
        std::string const call = std::string(__FUNCTION__) + " " + modifier;
        if (!modifierFuncTree->parseStr(call, jsonDoc)) {
            return false;
        }
    }
    return true;
}

std::string const JsonModifier::valueKey = "v";

//------------------------------------------
// Functions

bool JsonModifier::add(std::span<std::string const> const& args, JSON* jsonDoc) {
    auto numbers = args.subspan(1); // First argument is the modifier name
    for (auto const& numStr : numbers) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_add(valueKey, num);
        } catch (const std::invalid_argument&) {
            return false;
        } catch (const std::out_of_range&) {
            return false;
        }
    }
    return true;
}

}   // namespace Nebulite::Utility