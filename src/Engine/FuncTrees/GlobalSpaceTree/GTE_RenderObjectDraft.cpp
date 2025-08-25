#include "GTE_RenderObjectDraft.h"
#include "GlobalSpace.h"



void Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::update() {
    // For on-tick-updates

    // Init draft if not available
    // Putting this inside setupBindings is UB on windows build, which is why we do it here
    if(!draft) {
        draft = std::make_unique<Nebulite::RenderObject>(&domain->global);
    }
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::draftHelp(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS; // No arguments expected
    }

    // Implementation of draftHelp
    std::string command = "Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::draftHelp help";

    // Add additional arguments
    for(int i = 2; i < argc; ++i) {
        command += " ";
        command += argv[i];
    }

    // Parse the command
    return draft->parseStr(command);
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::onDraft(int argc, char* argv[]) {
    if(argc < 2){
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }

    // Implementation of onDraft
    std::string command;
    for (int i = 1; i < argc; ++i) {    // Ignoring first 2 argc: <from> <thisFunctionsName>
        command += argv[i];
        if (i < argc - 1) command += " ";
    }
    return draft->parseStr("Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::onDraft " + command);
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::spawnDraft(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS; // No arguments expected
    }

    // Turning Renderobject into string serial and parsing:
    // argv[0] : <from>
    // argv[1] : spawn
    // argv[2] : <serial>
    std::string serial = draft->serialize();
    char** argv_new = new char*[3];
    argv_new[0] = const_cast<char*>("Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::spawnDraft");
    argv_new[1] = const_cast<char*>("spawn");
    argv_new[2] = const_cast<char*>(serial.c_str());

    // Parse in globalspace
    funcTree->parse(3, argv_new);

    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::resetDraft(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS; // No arguments expected
    }

    Nebulite::RenderObject newDraft(&domain->global);
    draft->deserialize(newDraft.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}