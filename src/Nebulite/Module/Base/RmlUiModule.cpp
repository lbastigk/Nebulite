//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_events.h>

// Nebulite
#include "Nebulite/Graphics/RmlUi/Interface.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::Io::Capture& c, Graphics::RmlUi::Interface& i) :
    capture(c),
    interface(i)
{}

void RmlUiModule::update() {}

void RmlUiModule::postRenderUpdate() {}

void RmlUiModule::processRmlUiEvent(SDL_Event const& /*event*/, int const /*keyModifiers*/, Rml::Element* /*focusElement*/) {}

} // namespace Nebulite::Module::Base
