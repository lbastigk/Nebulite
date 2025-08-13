#include "GTE_RenderObjectDraft.h"

void Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::update() {
    // Do nothing
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::mockHelp(int argc, char* argv[]) {
    // Implementation of mockHelp
    mock.parseStr("Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::mockHelp help");
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::onMock(int argc, char* argv[]) {
    if(argc < 3){
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }

    // Implementation of onMock
    std::string command;
    for (int i = 1; i < argc; ++i) {    // Ignoring first 2 argc: <from> <thisFunctionsName>
        command += argv[i];
        if (i < argc - 1) command += " ";
    }
    return mock.parseStr("Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::onMock " + command);
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::spawnMock(int argc, char* argv[]) {
    // Turning Renderobject into string serial and parsing:
    // argv[0] : <from>
    // argv[1] : spawn
    // argv[2] : <serial>
    std::string serial = mock.serialize();
    char** argv_new = new char*[3];
    argv_new[0] = const_cast<char*>("Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::spawnMock");
    argv_new[1] = const_cast<char*>("spawn");
    argv_new[2] = const_cast<char*>(serial.c_str());

    // Parse in globalspace
    funcTree->parse(3, argv_new);

    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft::resetMock(int argc, char* argv[]) {
    Nebulite::RenderObject newMock;
    mock.deserialize(newMock.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}