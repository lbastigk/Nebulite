//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Module/Domain/Renderer/RenderObjectDraft.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event RenderObjectDraft::updateHook() {
    return draft.get(domain.capture).update();
}

Constants::Event RenderObjectDraft::draftParse(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const command = Utility::StringHandler::recombineArgs(args.subspan(1));
    return draft.get(domain.capture).parseStr(__FUNCTION__ + std::string(" ") + command, ctx, ctxScope);
}

Constants::Event RenderObjectDraft::draftSpawn() {
    // Make a copy of the draft's serialized data
    // Create a new RenderObject on the heap and append it to the renderer
    std::string const serial = draft.get(domain.capture).serialize();
    auto* newObj = new Core::RenderObject(domain.capture);
    newObj->deserialize(serial);
    domain.append(newObj);
    return Constants::Event::success;
}

Constants::Event RenderObjectDraft::draftReset() {
    Core::RenderObject const newDraft(domain.capture);
    draft.get(domain.capture).deserialize(newDraft.serialize());
    return Constants::Event::success;
}

RenderObjectDraft::RenderObjectDraft(ConstructorParams const& params) : DomainModule(params) {
    // Bind functions
    bindCategory(draftName, draftDesc);
    bindFunction(&RenderObjectDraft::draftParse, draftParseName, draftParseDesc);
    bindFunction(&RenderObjectDraft::draftSpawn, draftSpawnName, draftSpawnDesc);
    bindFunction(&RenderObjectDraft::draftReset, draftResetName, draftResetDesc);
}

} // namespace Nebulite::Module::Domain::Renderer
