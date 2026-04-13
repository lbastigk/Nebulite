#include "Module/Base/RmlUiModule.hpp"
#include "Nebulite.hpp"

namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::Capture& c, Core::Renderer& r) : accessToken(ScopeAccessor::Full()), capture(c), renderer(r) {
}

void RmlUiModule::update() {};

} // namespace Nebulite::Module::Base
