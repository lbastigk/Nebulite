#ifndef NEBULITE_MODULE_RMLUI_CONTEXT_MANAGER_HPP
#define NEBULITE_MODULE_RMLUI_CONTEXT_MANAGER_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"
#include <RmlUi/Core.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Utility/Capture.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ContextManager final : public Base::RmlUiModule {
public:
    explicit ContextManager(Utility::Capture& c, Core::Renderer& r);

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

    std::vector<Rml::Element*> toRegister;
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_CONTEXT_MANAGER_HPP
