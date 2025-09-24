#include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.hpp"
#include "Core/GlobalSpace.hpp"



void Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::update() {
    // For on-tick-updates

    // Init draft if not available
    // Putting this inside setupBindings is UB on windows build, which is why we do it here
    if(!draft) {
        draft = std::make_unique<Nebulite::Core::RenderObject>(domain);
    }
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::parse(int argc, char* argv[]) {
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string command;
    for (int i = 1; i < argc; ++i) {    // Ignoring first 2 argc: <from> <thisFunctionsName>
        command += argv[i];
        if (i < argc - 1) command += " ";
    }
    return draft->parseStr("Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::onDraft " + command);
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::spawnDraft(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    // Turning Renderobject into string serial and parsing:
    // argv[0] : <from>
    // argv[1] : spawn
    // argv[2] : <serial>
    std::string serial = draft->serialize();
    char** argv_new = new char*[3];
    argv_new[0] = const_cast<char*>("Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::spawnDraft");
    argv_new[1] = const_cast<char*>("spawn");
    argv_new[2] = const_cast<char*>(serial.c_str());

    // Parse in globalspace
    funcTree->parse(3, argv_new);

    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::RenderObjectDraft::resetDraft(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    Nebulite::Core::RenderObject newDraft(domain);
    draft->deserialize(newDraft.serialize());
    return Nebulite::Constants::ErrorTable::NONE();
}