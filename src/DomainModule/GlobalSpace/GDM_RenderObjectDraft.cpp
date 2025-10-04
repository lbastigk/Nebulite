#include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

void RenderObjectDraft::update() {}

Nebulite::Constants::Error RenderObjectDraft::draft_parse(int argc, char* argv[]) {
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string command;
    for (int i = 1; i < argc; ++i) {    // Ignoring first 2 argc: <from> <thisFunctionsName>
        command += argv[i];
        if (i < argc - 1) command += " ";
    }
    return draft.get()->parseStr(__FUNCTION__ + std::string(" ") + command);
}
const std::string RenderObjectDraft::draft_parse_name = "draft parse";
const std::string RenderObjectDraft::draft_parse_desc = R"(Parse Renderobject-specific functions on the draft.

Usage: draft parse <function> [args...]

Use 'draft parse help' to see available functions.

Examples:

draft parse set text.str Hello World
draft parse set posX 100
draft parse set posY 200
)";

Nebulite::Constants::Error RenderObjectDraft::draft_spawn(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    // Make a copy of the draft's serialized data
    // Create a new RenderObject on the heap and append it to the renderer
    std::string serial = draft.get()->serialize();
    Nebulite::Core::RenderObject* newObj = new Nebulite::Core::RenderObject(domain);
    newObj->deserialize(serial);
    domain->getRenderer()->append(newObj);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string RenderObjectDraft::draft_spawn_name = "draft spawn";
const std::string RenderObjectDraft::draft_spawn_desc = R"(Spawn the created draft object.

Usage: draft spawn
)";

Nebulite::Constants::Error RenderObjectDraft::draft_reset(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    Nebulite::Core::RenderObject newDraft(domain);
    draft.get()->deserialize(newDraft.serialize());
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string RenderObjectDraft::draft_reset_name = "draft reset";
const std::string RenderObjectDraft::draft_reset_desc = R"(Reset the draft object.

This does not reset any spawned ones!

Usage: draft reset
)";

} // namespace Nebulite::DomainModule::GlobalSpace