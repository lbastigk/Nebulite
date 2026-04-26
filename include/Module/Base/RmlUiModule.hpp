#ifndef NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP
#define NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>

// Nebulite
#include "Core/Renderer.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {
class RmlUiModule : public Rml::Plugin {
public:
    explicit RmlUiModule(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    virtual void update();

    virtual void postRenderUpdate();

protected:

    Utility::IO::Capture& capture;

    Graphics::RmlInterface& interface;
};
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP
