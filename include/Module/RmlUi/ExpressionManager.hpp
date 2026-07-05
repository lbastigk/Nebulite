#ifndef MODULE_RMLUI_EXPRESSIONMANAGER_HPP
#define MODULE_RMLUI_EXPRESSIONMANAGER_HPP

//------------------------------------------
// Includes

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
// Forward declarations

namespace Rml {
class Element;
} // namespace Rml

namespace Nebulite::Graphics {
class RmlInterface;
} // namespace Nebulite::Graphics

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ExpressionManager final : public Base::RmlUiModule {
public:
    explicit ExpressionManager(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void postRenderUpdate() override;

    struct Attribute {
        static auto constexpr eval = "data-eval";
        static auto constexpr conditional = "data-if";

        static bool hasSupportedAttribute(Rml::Element* element) {
            return element->GetAttribute(eval) || element->GetAttribute(conditional);
        }
    };

private:
    volatile bool expressionsWereEvaluated = false;

    // Pre-compiled RML strings to expressions
    absl::flat_hash_map<
        Rml::String,
        Interaction::Logic::Expression
    > expressions;

    // Original, non-evaluated rml strings of elements
    absl::flat_hash_map<
        Rml::Element*,
        Rml::String
    > rmlStrings;

   // std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void updateExpressions();

    void resetExpressions();

    Utility::Coordination::TimedRoutine evaluationRoutine{[this] {
            updateExpressions();
            expressionsWereEvaluated = true;
        },
        // If 1000.0/fps is higher than this value, the ui starts glitching due to the reset rml still being written while rendering.
        // So we update Expressions instantly with each new render pass.
        // Getting rid of the TimedRoutine is also an option, but we leave it here atm.
        0,
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    };
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_EXPRESSIONMANAGER_HPP
