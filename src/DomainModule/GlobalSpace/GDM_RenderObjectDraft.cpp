#include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.hpp"
#include "Core/GlobalSpace.hpp"



void Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::update() {}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::draft_parse(int argc, char* argv[]) {
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string command;
    for (int i = 1; i < argc; ++i) {    // Ignoring first 2 argc: <from> <thisFunctionsName>
        command += argv[i];
        if (i < argc - 1) command += " ";
    }
    return draft.get()->parseStr("Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::onDraft " + command);
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::draft_spawn(int argc, char* argv[]) {
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

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::draft_reset(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }
    Nebulite::Core::RenderObject newDraft(domain);
    draft.get()->deserialize(newDraft.serialize());
    return Nebulite::Constants::ErrorTable::NONE();
}