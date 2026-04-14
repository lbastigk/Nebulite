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

    // TODO: Add attribute-overwrite attribute to set any reflect element to the reflection id or some string. This is important for unique identifiers

    struct ReflectionEntry {
        Interaction::Logic::Expression entries;
        Interaction::ContextScope const context;
        Rml::String rmlValue;
        Data::JSON jsonResult;
        bool markedForDeletion = false;
    };

    absl::flat_hash_map<
        Rml::ElementDocument*,
        absl::flat_hash_map<
            Rml::Element*,
            ReflectionEntry
        >
    >reflections;

    std::vector<std::pair<Rml::Element*, ReflectionEntry>> reflectOnce;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    static std::string modifyDataIdentifier(std::string const& input, size_t const& index);

    void removeDeletedElements();

    void reflect();

    void reflectElement(Rml::Element* element, ReflectionEntry& entry) const ;
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_REFLECTION_HPP
