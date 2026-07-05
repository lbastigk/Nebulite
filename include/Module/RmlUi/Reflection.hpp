#ifndef MODULE_RMLUI_REFLECTION_HPP
#define MODULE_RMLUI_REFLECTION_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>
#include <utility>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
// Forward declarations

namespace Rml {
class Element;
class ElementDocument;
} // namespace Rml

namespace Nebulite::Graphics {
class RmlInterface;
} // namespace Nebulite::Graphics

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

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
        std::vector<Graphics::RmlInterface::RmlElementIdentifier> allocatedIds; // Instead of constantly allocating new element Identifiers per reflection, we reuse them.
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
     * @param entry The reflectionEntry
     * @param element The rml element associated with the reflection
     * @param contextAndScope The context and scope of the element
     */
    void setReflectionScopes(Data::JSON& reflectionList, std::unique_ptr<ReflectionEntry> const& entry, Rml::Element const* element, Graphics::RmlInterface::ContextAndScope const& contextAndScope) const ;

    /**
     * @brief Reflects a single element based on the provided entry
     * @param element The rml element to modify
     * @param entry The reflection entry
     */
    void reflectElement(Rml::Element* element, std::unique_ptr<ReflectionEntry> const& entry);

    /**
     * @brief Sets the identifier of given element and all its children, recursively
     * @param element The element to modify
     * @param id The identifier to use for the element and all its children
     */
    static void setIdentifiers(Rml::Element* element, Graphics::RmlInterface::RmlElementIdentifier const& id);
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_REFLECTION_HPP
