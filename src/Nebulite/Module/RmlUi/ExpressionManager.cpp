//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Nebulite/Graphics/RmlUi/ElementIdentifier.hpp"
#include "Nebulite/Graphics/RmlUi/Interface.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Module/RmlUi/ExpressionManager.hpp"
#include "Nebulite/Utility/Coordination/TimedRoutine.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ExpressionManager::ExpressionManager(Utility::Io::Capture& c, Graphics::RmlUi::Interface& i) : RmlUiModule(c,i) {}

void ExpressionManager::update() {
    evaluationRoutine.update();
}

void ExpressionManager::postRenderUpdate() {
    if (expressionsWereEvaluated) {
        resetExpressions();
        expressionsWereEvaluated = false;
    }
}

//----------------------------------------------

void ExpressionManager::updateExpressions(){
    for (auto const& document : interface.getOpenedDocuments()) {
        Graphics::RmlUi::Interface::updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/) {
            if (Attribute::hasSupportedAttribute(element)) {
                // On element creation, the inner rml is not set. So we create an empty ElementEntry that is populated later on.
                Rml::String innerRml = element->GetInnerRML();
                rmlStrings[element] = innerRml;
                if (auto const it = expressions.find(innerRml); it == expressions.end()) {
                    expressions.emplace(innerRml, Interaction::Logic::Expression(innerRml));
                }

                Graphics::RmlUi::ElementIdentifier const elementId(element);
                if (auto const context = interface.getRmlElementContextAndScope(elementId); context.has_value()) {
                    if (context.value().ctxScope.hasDummyScope()) {
                        capture.warning.println("Failed to evaluate expression, a context member has a dummy scope!");
                        return;
                    }
                    if (auto const it = expressions.find(innerRml); it != expressions.end()) {
                        std::string const& evaluated = it->second.eval(context.value().ctxScope);
                        element->SetInnerRML(evaluated);
                    }
                }
            }
        });
    }
}

void ExpressionManager::resetExpressions(){
    for (auto const& document : interface.getOpenedDocuments()) {
        Graphics::RmlUi::Interface::updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/) {
            if (Attribute::hasSupportedAttribute(element)) {
                // Reset
                element->SetInnerRML(rmlStrings[element]);
            }
        });
    }
}

} // namespace Nebulite::Module::RmlUi
