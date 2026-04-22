#include "Module/Base/RmlUiModule.hpp"
#include "Nebulite.hpp"

namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::IO::Capture& c, Core::Renderer& r) :
    capture(c),
    renderer(r),
    global(Global::shareScope(ScopeAccessor::Full()))
{}

void RmlUiModule::update() {}

void RmlUiModule::postRenderUpdate() {}

void RmlUiModule::updateElement(Rml::Element* element, std::function<void(Rml::Element*, Rml::Element*, size_t const&)> const& updateFunc) {
    size_t const numChildren = static_cast<size_t>(element->GetNumChildren());
    for (size_t i = 0; i < numChildren; ++i) {
        if (auto const child = element->GetChild(static_cast<int>(i)); child) {
            updateFunc(child, element, i);
            updateElement(child, updateFunc);
        }
    }
}

} // namespace Nebulite::Module::Base
