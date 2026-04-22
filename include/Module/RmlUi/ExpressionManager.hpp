#ifndef NEBULITE_MODULE_RMLUI_EXPRESSION_MANAGER_HPP
#define NEBULITE_MODULE_RMLUI_EXPRESSION_MANAGER_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"
#include <RmlUi/Core.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class ExpressionManager final : public Base::RmlUiModule {
public:

    explicit ExpressionManager(Utility::IO::Capture& c, Core::Renderer& r);

    void update() override ;

    void postRenderUpdate() override;

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

    bool expressionsWereEvaluated = false;

    std::vector<Rml::ElementDocument*> documents;

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

    void updateExpressions();

    //--------------------------------

    struct RegisteredEntry {
        Rml::Element* element = nullptr;
        Rml::String currentRmlValue;
        Rml::String previousRmlValue;
        std::string previousDocumentValue;
    };

    absl::node_hash_map<std::string, std::unique_ptr<RegisteredEntry>> registeredStrings;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void updateDataValues();

    void resetExpressions();

};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_EXPRESSION_MANAGER_HPP
