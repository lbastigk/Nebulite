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

    virtual void postRenderUpdate();

protected:

    Utility::Capture& capture;

    Core::Renderer& renderer;

    Data::JsonScope& global;

    static void updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*, size_t const&)> const& updateFunc);
};
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_RML_UI_MODULE_HPP
