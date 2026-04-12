#ifndef RML_TEST_PLUGIN_HPP
#define RML_TEST_PLUGIN_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include "absl/container/flat_hash_map.h"
#include <RmlUi/Core.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Utility/Capture.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
namespace Nebulite::UI::Plugin {

class TestPlugin final : public Module::Base::RmlUiModule {
public:
    explicit TestPlugin(Utility::Capture& c) : RmlUiModule(c) {}

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

    absl::flat_hash_map<
        Rml::ElementDocument*,
        absl::flat_hash_map<
            Rml::Element*,
            Interaction::Logic::Expression
        >
    >expressions;

    void compileDocument(Rml::ElementDocument* root, Rml::Element* element, size_t const& depth);
};

} //

#endif // RML_TEST_PLUGIN_HPP
