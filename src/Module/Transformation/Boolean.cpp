//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Transformation/Boolean.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Boolean::bindTransformations() {
    bindTransformation(&Boolean::booleanNot, booleanNotName, booleanNotDesc);
}

bool Boolean::booleanNot(Data::JsonScope& jsonDoc){
    if (auto const val = jsonDoc.get<bool>(rootKey); !val) {
        // Assume value is false, set to true
        jsonDoc.set<bool>(rootKey, true);
    }
    else {
        // Invert
        jsonDoc.set<bool>(rootKey, !val.value());
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
