//------------------------------------------
// Includes

// Nebulite
#include "Core/Renderer.hpp"
#include "Module/Domain/Renderer/RenderObjectDraft.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event RenderObjectDraft::updateHook() {
    return draft.get(domain.capture)->update();
}

Constants::Event RenderObjectDraft::draft_parse(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const command = Utility::StringHandler::recombineArgs(args.subspan(1));
    return draft.get(domain.capture)->parseStr(__FUNCTION__ + std::string(" ") + command, ctx, ctxScope);
}

Constants::Event RenderObjectDraft::draft_spawn() {
    // Make a copy of the draft's serialized data
    // Create a new RenderObject on the heap and append it to the renderer
    std::string const serial = draft.get(domain.capture)->serialize();
    auto* newObj = new Core::RenderObject(domain.capture);
    newObj->deserialize(serial);
    domain.append(newObj);
    return Constants::Event::Success;
}

Constants::Event RenderObjectDraft::draft_reset() {
    Core::RenderObject const newDraft(domain.capture);
    draft.get(domain.capture)->deserialize(newDraft.serialize());
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
