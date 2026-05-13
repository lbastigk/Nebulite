//------------------------------------------
// Includes

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::IO::Capture& c, Graphics::RmlInterface& i) :
    capture(c),
    interface(i)
{}

void RmlUiModule::update() {}

void RmlUiModule::postRenderUpdate() {}

} // namespace Nebulite::Module::Base
