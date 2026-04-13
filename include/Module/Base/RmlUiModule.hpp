#ifndef NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP
#define NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>

// Nebulite
#include "Core/Renderer.hpp"
#include "ScopeAccessor.hpp"
#include "Utility/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {
class RmlUiModule : public Rml::Plugin {
public:
    explicit RmlUiModule(Utility::Capture& c, Core::Renderer& r);

    virtual void update();

protected:

    Utility::Capture& capture;

    Core::Renderer& renderer;

    Data::JsonScope& global;
};
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP
