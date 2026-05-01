#ifndef NEBULITE_MODULE_RMLUI_REFLECTION_HPP
#define NEBULITE_MODULE_RMLUI_REFLECTION_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include <absl/container/flat_hash_map.h>
#include <RmlUi/Core.h>

// Nebulite
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

    static auto constexpr reflectionAttribute = "data-reflect";
    static auto constexpr reflectionOnceAttribute = "data-reflect-once";

private:
    /**
     * @brief Necessary metadata information for each entry
     */
    struct ReflectionEntry {
        Interaction::Logic::Expression reflectionListExpression; // Expression to generate an array of entries to reflect to
        Data::JSON reflectionList; // Result of evaluating the expression, must be an array
        Rml::String rmlValue; // Original RML value to replicate for each entry
        bool markedForDeletion = false;
        std::vector<size_t> allocatedIds; // Instead of constantly allocating new element Identifiers per reflection, we reuse them.
    };

    // All registered reflections
    absl::flat_hash_map<Rml::Element*, ReflectionEntry> reflections;

    // All one-time reflections
    std::vector<std::pair<Rml::Element*, ReflectionEntry>> reflectOnce;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void removeDeletedElements();

    void reflect();

    void reflectElement(Rml::Element* element, ReflectionEntry& entry) const ;
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_REFLECTION_HPP
