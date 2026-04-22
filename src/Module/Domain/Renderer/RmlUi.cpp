#include "Module/Domain/Renderer/RmlUi.hpp"

#include "Core/Renderer.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

namespace Nebulite::DomainModule::Renderer {

Constants::Event RmlUi::updateHook() {
    // No update tasks for now, but this can be used for future features such as RmlUi event handling or variable updates
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event RmlUi::loadDocument(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const& name = args[1];

    if (loadedDocuments.find(name) != loadedDocuments.end()) {
        domain.capture.warning.println("Document with name '", name, "' already exists. Please choose a different name or remove the existing document first.");
        return Constants::Event::Warning;
    }

    auto const document = Utility::IO::FileManagement::LoadFile(Utility::StringHandler::recombineArgs(args.subspan(2)));

    Rml::ElementDocument* doc = domain.getRmlContext()->LoadDocumentFromMemory(document);
    domain.setRmlDocumentContextAndScope(doc, {ctx, ctxScope});
    if (!doc) {
        domain.capture.warning.println("Failed to load document '", document, "'.");
        return Constants::Event::Warning;
    }
    doc->Show();
    loadedDocuments[name] = doc;
    return Constants::Event::Success;
}

Constants::Event RmlUi::removeDocument(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const& name = args[1];

    auto const it = loadedDocuments.find(name);
    if (it == loadedDocuments.end()) {
        domain.capture.warning.println("No document with name '", name, "' found. Please check the name and try again.");
        return Constants::Event::Warning;
    }

    it->second->Close();
    domain.getRmlContext()->UnloadDocument(it->second);
    loadedDocuments.erase(it);
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::Renderer
