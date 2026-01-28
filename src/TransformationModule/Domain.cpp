#include "TransformationModule/Domain.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Domain::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Domain::nebs, nebsName, nebsDesc);
}

bool Domain::nebs(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (jsonDoc->parseStr(Utility::StringHandler::recombineArgs(args)) != Constants::ErrorTable::NONE()) {
        return false;
    }
    return true;
}

} // namespace Nebulite::TransformationModule
