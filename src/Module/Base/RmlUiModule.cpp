#include "Module/Base/RmlUiModule.hpp"
#include "Nebulite.hpp"

namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::IO::Capture& c, Graphics::RmlInterface& i) :
    capture(c),
    interface(i)
{}

void RmlUiModule::update() {}

void RmlUiModule::postRenderUpdate() {}

} // namespace Nebulite::Module::Base
