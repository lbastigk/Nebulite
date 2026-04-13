#ifndef NEBULITE_MODULE_RMLUI_REFLECTION_HPP
#define NEBULITE_MODULE_RMLUI_REFLECTION_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include "absl/container/flat_hash_map.h"
#include <RmlUi/Core.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Utility/Capture.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class Reflection final : public Base::RmlUiModule {
public:
    explicit Reflection(Utility::Capture& c, Core::Renderer& r);

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

    // TODO: Separate module for simple input to variable sync: <input type="text" data-value="animal"/>
    //       Two-way sync would be nice.

    // TODO: set context for each document. Idea: Rml::ElementDocument* -> context in renderer, on document load via DomainModule, set context from caller scope
    //       we need to store the Nebulite Renderer reference for each RmlUiModule so we can easily access this data from any module (or use Global::getRenderer()?
    //       Then we can use another RmlUiModule just for context management on Document load/unload. If no context is available, use an dummy context (dummy scope for all scopes)

    struct ReflectionEntry {
        Interaction::Logic::Expression entries;
        Interaction::ContextScope context;
        Rml::String rmlValue;
        bool markedForDeletion = false;
    };

    absl::flat_hash_map<
        Rml::ElementDocument*,
        absl::flat_hash_map<
            Rml::Element*,
            ReflectionEntry
        >
    >reflections;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void removeDeletedElements();

    void reflect();
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_REFLECTION_HPP
