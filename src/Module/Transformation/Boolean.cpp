//------------------------------------------
// Includes

// Standard library
#include <string>
#include <variant>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/Boolean.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Boolean::bindTransformations() {
    bindTransformation(&Boolean::booleanNot, booleanNotName, booleanNotDesc);
}

bool Boolean::booleanNot(Data::JsonScope* jsonDoc){
    auto const val = jsonDoc->get<bool>(rootKey);
    if (!val) return false;
    jsonDoc->set<bool>(rootKey, !val.value());
    return true;
}

} // namespace Nebulite::Module::Transformation
