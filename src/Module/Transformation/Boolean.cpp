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
        auto const valOpt = jsonDoc->getVariant(rootKey);
        if (!valOpt.has_value()) {
            return false; // Null values cannot be processed
        }
        auto const& val = valOpt.value();
        bool result = false;
        if (std::holds_alternative<bool>(val)) {
            result = !std::get<bool>(val);
        } else if (std::holds_alternative<int>(val)) {
            result = std::get<int>(val) != 0;
        } else if (std::holds_alternative<double>(val)) {
            result = std::get<double>(val) != 0.0;
        } else if (std::holds_alternative<std::string>(val)) {
            auto const& strVal = std::get<std::string>(val);
            result = !(strVal == "true" || strVal == "1" || strVal == "yes" || strVal == "on");
        } else {
            // For other types (objects, arrays), we can define them as truthy or falsy as needed
            // For now, we will treat them as truthy
            result = true;
        }

        jsonDoc->set<bool>(rootKey, result);
        return true;
}

} // namespace Nebulite::Module::Transformation
