//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ExpressionManager::ExpressionManager(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

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
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/) {
            if (Attribute::hasSupportedAttribute(element)) {
                // On element creation, the inner rml is not set. So we create an empty ElementEntry that is populated later on.
                Rml::String innerRml = element->GetInnerRML();
                rmlStrings[element] = innerRml;
                if (auto const it = expressions.find(innerRml); it == expressions.end()) {
                    expressions.emplace(innerRml, Interaction::Logic::Expression(innerRml));
                }

                Graphics::RmlInterface::RmlElementIdentifier const elementId(element);
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
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/) {
            if (Attribute::hasSupportedAttribute(element)) {
                // Reset
                element->SetInnerRML(rmlStrings[element]);
            }
        });
    }
}

} // namespace Nebulite::Module::RmlUi
