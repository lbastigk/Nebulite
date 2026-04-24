#include "Module/Base/RmlUiModule.hpp"
#include "Nebulite.hpp"

namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::IO::Capture& c, Core::Renderer& r) :
    capture(c),
    renderer(r)
{}

void RmlUiModule::update() {}

void RmlUiModule::postRenderUpdate() {}

} // namespace Nebulite::Module::Base
