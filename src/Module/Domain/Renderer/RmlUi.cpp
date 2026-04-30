//------------------------------------------
// Includes

// Nebulite
#include "Core/Renderer.hpp"
#include "Module/Domain/Renderer/RmlUi.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event RmlUi::updateHook() {
    moduleScope.set<uint64_t>(Key::openedDocuments, domain.rmlDocumentCount());
    return Constants::Event::Success;
}

Constants::Event RmlUi::loadDocument(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const& name = args[1];
    auto path = Utility::StringHandler::recombineArgs(args.subspan(2));
    if (!domain.loadRmlDocument(name, path, ctx, ctxScope)) {
        domain.capture.warning.println("Failed to load document: '", path, "'. Either the owner already has a document with the same name, or the file could not be loaded. Please check the name and path, and try again.");
        return Constants::Event::Warning;
    }
    return Constants::Event::Success;
}

Constants::Event RmlUi::removeDocument(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const& name = args[1];
    if (!domain.removeRmlDocument(ctx.self.getId(), name)) {
        domain.capture.warning.println("Failed to remove document: '", name, "'. Either the owner does not have a document with this name, or there was an issue removing the document. Please check the name, and try again.");
        domain.capture.warning.println("Ensure that the context self is the actual owner of the document!");
        return Constants::Event::Warning;
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Renderer
