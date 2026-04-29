#ifndef NEBULITE_MODULE_RMLUI_CONTEXT_MANAGER_HPP
#define NEBULITE_MODULE_RMLUI_CONTEXT_MANAGER_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>

// Nebulite
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ContextManager final : public Base::RmlUiModule {
public:
    explicit ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void OnInitialise() override ;

    void OnShutdown() override ;

    void OnDocumentOpen(Rml::Context* context, const Rml::String& document_path) override ;

    void OnDocumentLoad(Rml::ElementDocument* document) override ;

    void OnDocumentUnload(Rml::ElementDocument* document) override ;

    void OnContextCreate(Rml::Context* context) override ;

    void OnContextDestroy(Rml::Context* context) override ;

    void OnElementCreate(Rml::Element* element) override ;

    void OnElementDestroy(Rml::Element* element) override ;

private:

    std::vector<Rml::ElementDocument*> documents;

    std::vector<Rml::Element*> toRegister;
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_CONTEXT_MANAGER_HPP
