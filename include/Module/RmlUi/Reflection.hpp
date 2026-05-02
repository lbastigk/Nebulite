#ifndef NEBULITE_MODULE_RMLUI_REFLECTION_HPP
#define NEBULITE_MODULE_RMLUI_REFLECTION_HPP

//------------------------------------------
// Includes

// External
#include <absl/container/flat_hash_map.h>
#include <RmlUi/Core.h>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class Reflection final : public Base::RmlUiModule {
public:
    explicit Reflection(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void OnElementCreate(Rml::Element* element) override ;

    void OnElementDestroy(Rml::Element* element) override ;

    void OnDocumentLoad(Rml::ElementDocument* document) override {
        reflectionResults.emplace(document, absl::flat_hash_map<Rml::Element*, std::unique_ptr<Data::JSON>>{});
    }

    void OnDocumentUnload(Rml::ElementDocument* document) override {
        reflectionResults.erase(document);
    }

    static auto constexpr reflectionAttribute = "data-reflect";
    static auto constexpr reflectionOnceAttribute = "data-reflect-once";

private:
    /**
     * @brief Necessary metadata information for each entry
     */
    struct ReflectionEntry {
        ReflectionEntry(std::string const& expression, Rml::Element const* element) : reflectionListExpression(expression) {
            rmlValue = element->GetInnerRML();
        }

        Interaction::Logic::Expression reflectionListExpression; // Expression to generate an array of entries to reflect to
        Rml::String rmlValue; // Original RML value to replicate for each entry
        bool markedForDeletion = false;
        std::vector<size_t> allocatedIds = {}; // Instead of constantly allocating new element Identifiers per reflection, we reuse them.
    };

    // Entries that we have to add to the active list
    struct ToAdd {
        std::vector<std::pair<Rml::Element*, std::unique_ptr<ReflectionEntry>>> reflections;
        std::vector<std::pair<Rml::Element*, std::unique_ptr<ReflectionEntry>>> reflectOnce;
    } toAdd;

    // Reflection results
    // Must be kept alive as long as the document is alive
    // Technically, this could be stored in the ReflectionEntry.
    // The idea with this container was to allow nested reflections,
    // and this setup solved some details. However, a bigger overhaul of the reflection system
    // is necessary for this to work.
    absl::flat_hash_map<
        Rml::ElementDocument*,
        absl::flat_hash_map<
            Rml::Element*,
            std::unique_ptr<Data::JSON>
        >
    >
    reflectionResults;

    // All registered reflections
    absl::flat_hash_map<Rml::Element*, std::unique_ptr<ReflectionEntry>> reflections;

    // All one-time reflections
    std::vector<std::pair<Rml::Element*, std::unique_ptr<ReflectionEntry>>> reflectOnce;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void removeDeletedElements();

    void reflect();

    void reflectElement(Rml::Element* element, std::unique_ptr<ReflectionEntry> const& entry);
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_REFLECTION_HPP
