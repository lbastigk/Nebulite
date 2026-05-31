#ifndef MODULE_RMLUI_REFLECTION_HPP
#define MODULE_RMLUI_REFLECTION_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/Capture.hpp"

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

    struct Attribute {
        static auto constexpr reflect = "data-reflect"; // Continuous reflection and evaluation
        static auto constexpr reflectOne = "data-reflect-once"; // Reflect and evaluate once

        static bool hasSupportedAttribute(Rml::Element* element) {
            return element->GetAttribute(reflect) || element->GetAttribute(reflectOne);
        }
    };

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
        std::vector<size_t> allocatedIds; // Instead of constantly allocating new element Identifiers per reflection, we reuse them.
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

    /**
     * @brief Removes all entries that are marked for deletion from the active reflections list
     */
    void removeDeletedElements();

    /**
     * @brief Modifies the elements of all registered reflection entries
     */
    void reflect();

    /**
     * @brief Evaluates the expression of the ReflectionEntry
     * @param entry The entry to evaluate the expression of
     * @param element The rml element associated with the reflection
     * @param scope The scope of the element
     * @return The result of the expression
     */
    Data::JSON& evaluateReflectionList(std::unique_ptr<ReflectionEntry> const& entry, Rml::Element* element, Interaction::ContextScope const& scope);

    /**
     * @brief Sets the scopes for all parts of a reflection entry
     * @param reflectionList The list of values to use for each reflection
     * @param listSize Size of reflectionList (using memberSize is costly. Since it's already known, we just pass it)
     * @param entry The reflectionEntry
     * @param element The rml element associated with the reflection
     * @param context The context of the element
     * @param scope The scope of the element
     */
    void setReflectionScopes(
        Data::JSON& reflectionList,
        size_t const& listSize,
        std::unique_ptr<ReflectionEntry> const& entry,
        Rml::Element const* element,
        Interaction::Context const& context,
        Interaction::ContextScope const& scope
    ) const ;

    /**
     * @brief Reflects a single element based on the provided entry
     * @param element The rml element to modify
     * @param entry The reflection entry
     */
    void reflectElement(Rml::Element* element, std::unique_ptr<ReflectionEntry> const& entry);
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_REFLECTION_HPP
