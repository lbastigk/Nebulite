#include "Module/Base/RmlUiModule.hpp"
#include "Nebulite.hpp"

namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::Capture& c) : accessToken(ScopeAccessor::Full()), capture(c) {
}

void RmlUiModule::update() {};

} // namespace Nebulite::Module::Base
