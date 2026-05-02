//------------------------------------------
// Includes

// Standard library
#include <regex>

// Nebulite
#include "Nebulite.hpp"
#include "Module/Transformation/Sort.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Sort::bindTransformations(){
    bindCategory(sortName, sortDesc);
    bindTransformation(&Sort::sortCaseSensitive,sortCaseSensitiveName, sortCaseSensitiveDesc);
    bindTransformation(&Sort::sortCaseInsensitive, sortCaseInsensitiveName, sortCaseInsensitiveDesc);
    bindTransformation(&Sort::sortNumerically, sortNumericallyName, sortNumericallyDesc);
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

} // namespace Nebulite::Module::Transformation
