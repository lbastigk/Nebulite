#include "TransformationModule/Assertions.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Assertions::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Assertions::assertNonEmpty, assertNonEmptyName, assertNonEmptyDesc);
}

// NOLINTNEXTLINE
bool Assertions::assertNonEmpty(Core::JsonScope* jsonDoc) {
    static std::string errorMessage = std::string(__FUNCTION__) + " JSON value is null";

    if (jsonDoc->memberType(valueKey) == Data::KeyType::null) {
        throw std::runtime_error(errorMessage);
        //return false;
    }
    return true;
}

} // namespace Nebulite::TransformationModule
