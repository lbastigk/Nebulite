#ifndef MODULE_RMLUI_EXPRESSIONMANAGER_HPP
#define MODULE_RMLUI_EXPRESSIONMANAGER_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ExpressionManager final : public Base::RmlUiModule {
public:
    explicit ExpressionManager(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void postRenderUpdate() override;

    static auto constexpr evalAttribute = "data-eval";
    static auto constexpr conditionalAttribute = "data-if";

private:
    bool expressionsWereEvaluated = false;

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

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void updateExpressions();

    void resetExpressions();
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_EXPRESSIONMANAGER_HPP
