#include "DomainModule/Renderer/RenderObjectDraft.hpp"
#include "Core/Renderer.hpp"

namespace Nebulite::DomainModule::Renderer {

std::string const RenderObjectDraft::draft_name = "draft";
std::string const RenderObjectDraft::draft_desc = R"(Functions to manipulate and spawn RenderObjects in draft state)";

Constants::Error RenderObjectDraft::update(){ 
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error RenderObjectDraft::draft_parse(int argc,  char** argv){
    if(argc < 2){
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const command = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    return draft.get()->parseStr(__FUNCTION__ + std::string(" ") + command);
}
std::string const RenderObjectDraft::draft_parse_name = "draft parse";
std::string const RenderObjectDraft::draft_parse_desc = R"(Parse Renderobject-specific functions on the draft.

Usage: draft parse <function> [args...]

Use 'draft parse help' to see available functions.

Examples:

draft parse set text.str Hello World
draft parse set posX 100
draft parse set posY 200
)";

// NOLINTNEXTLINE
Constants::Error RenderObjectDraft::draft_spawn(int argc,  char** argv){
    if(argc != 1){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    // Make a copy of the draft's serialized data
    // Create a new RenderObject on the heap and append it to the renderer
    std::string const serial = draft.get()->serialize();
    auto* newObj = new Core::RenderObject();
    newObj->deserialize(serial);
    domain->append(newObj);
    return Constants::ErrorTable::NONE();
}
std::string const RenderObjectDraft::draft_spawn_name = "draft spawn";
std::string const RenderObjectDraft::draft_spawn_desc = R"(Spawn the created draft object.

Usage: draft spawn
)";

// NOLINTNEXTLINE
Constants::Error RenderObjectDraft::draft_reset(int argc,  char** argv){
    if(argc != 1){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    Core::RenderObject newDraft;
    draft.get()->deserialize(newDraft.serialize());
    return Constants::ErrorTable::NONE();
}
std::string const RenderObjectDraft::draft_reset_name = "draft reset";
std::string const RenderObjectDraft::draft_reset_desc = R"(Reset the draft object.

This does not reset any spawned ones!

Usage: draft reset
)";

} // namespace Nebulite::DomainModule::GlobalSpace