#include "Module/Base/RmlUiModule.hpp"
#include "Nebulite.hpp"

namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::Capture& c, Core::Renderer& r) :
    capture(c),
    renderer(r),
    global(Global::shareScope(ScopeAccessor::Full()))
{}

void RmlUiModule::update() {}

} // namespace Nebulite::Module::Base
