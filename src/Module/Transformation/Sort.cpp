//------------------------------------------
// Includes

// Standard library
#include <regex>

// Nebulite
#include "Module/Transformation/Sort.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Sort::bindTransformations(){
    bindCategory(sortName, sortDesc);
    bindTransformation(&Sort::sortCaseSensitive,sortCaseSensitiveName, sortCaseSensitiveDesc);
    bindTransformation(&Sort::sortCaseInsensitive, sortCaseInsensitiveName, sortCaseInsensitiveDesc);
    bindTransformation(&Sort::sortNumerically, sortNumericallyName, sortNumericallyDesc);
    bindTransformation(&Sort::sortCustom, sortCustomName, sortCustomDesc);
}

bool Sort::sortCaseSensitive(Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    sort<std::string>(jsonDoc, "", [](auto const& a, auto const& b) {
        return Utility::Sort::caseSensitiveLess(a.first, b.first);
    });
    return true;
}

bool Sort::sortCaseInsensitive(Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    sort<std::string>(jsonDoc, "", [](auto const& a, auto const& b) {
        return Utility::Sort::caseInsensitiveLess(a.first, b.first);
    });
    return true;
}

bool Sort::sortNumerically(Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    sort<double>(jsonDoc,0, [](auto const& a, auto const& b) {
        return a.first < b.first;
    });
    return true;
}

bool Sort::sortCustom(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (jsonDoc->memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    if (args.size() < 2) return false;
    Interaction::Logic::Expression const expression(Utility::StringHandler::recombineArgs(args.subspan(1)));
    sort<bool>(jsonDoc, false, [&](auto& a, auto& b) {
        auto& slf = a.second.shareManagedScopeBase("");
        auto& otr = b.second.shareManagedScopeBase("");
        Interaction::ContextScope const ctxScope{
            {
                .self = slf,
                .other = otr,
                .global = slf
            }
        };
        return expression.evalAsBool(ctxScope);
    });
    return true;
}

} // namespace Nebulite::Module::Transformation
