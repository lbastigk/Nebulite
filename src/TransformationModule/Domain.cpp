#include "TransformationModule/Domain.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Domain::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Domain::parse, parseName, parseDesc);
}

bool Domain::parse(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->parseStr(__FUNCTION__ + std::string(" ") + extractPotentiallyWrappedString(args.subspan(1))) != Constants::ErrorTable::NONE()) {
        return false;
    }
    return true;
}

} // namespace Nebulite::TransformationModule
