#ifndef RML_TEST_PLUGIN_HPP
#define RML_TEST_PLUGIN_HPP

#include "absl/container/flat_hash_map.h"
#include <RmlUi/Core.h>

#include "Interaction/Logic/Expression.hpp"
#include "Utility/Capture.hpp"

// TODO: custom plugin base class with virtual update() function + all standard virtual Rml::Plugin functions.
namespace Nebulite::UI::Plugin {

class TestPlugin final : public Rml::Plugin {
public:
    explicit TestPlugin(Utility::Capture& c) : capture(c) {}

    void update();

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
    Utility::Capture& capture; // Could be helpful for logging and error handling within the plugin.

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
