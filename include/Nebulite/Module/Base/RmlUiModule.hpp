#ifndef NEBULITE_MODULE_BASE_RMLUIMODULE_HPP
#define NEBULITE_MODULE_BASE_RMLUIMODULE_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Plugin.h>
#include <SDL3/SDL_events.h>

// Nebulite
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
// Forward declarations

namespace Rml {
class Context;
class Element;
class ElementDocument;
} // namespace Rml

//------------------------------------------
namespace Nebulite::Module::Base {
class RmlUiModule : public Rml::Plugin {
public:
    explicit RmlUiModule(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    virtual void update();

    virtual void postRenderUpdate();

    virtual void processRmlUiEvent(SDL_Event const& event, int keyModifiers, Rml::Element* focusElement);

    void OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) override {}

    void OnDocumentLoad(Rml::ElementDocument* /*document*/) override {}

    void OnDocumentUnload(Rml::ElementDocument* /*document*/) override {}

protected:
    Utility::IO::Capture& capture;

    Graphics::RmlInterface& interface;
};
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_RMLUIMODULE_HPP
