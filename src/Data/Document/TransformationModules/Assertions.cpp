#include "Data/Document/TransformationModules/Assertions.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Data::TransformationModules {

void Assertions::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Assertions::assertNonEmpty, assertNonEmptyName, assertNonEmptyDesc);
}

bool Assertions::assertNonEmpty(Core::JsonScope* jsonDoc) {
    static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is null";

    if (jsonDoc->memberType(valueKey) == KeyType::null) {
        throw std::runtime_error(errorMessage);
        //return false;
    }
    return true;
}

} // namespace Nebulite::Data::TransformationModules
