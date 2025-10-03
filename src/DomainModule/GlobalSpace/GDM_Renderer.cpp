#include "DomainModule/GlobalSpace/GDM_Renderer.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite
#include "Interaction/Invoke.hpp"            // Invoke for parsing expressions
#include "Core/RenderObject.hpp"      // RenderObject for Renderer

//------------------------------------------
// Update
void Nebulite::DomainModule::GlobalSpace::Renderer::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::env_load(int argc, char* argv[]){
    if(argc > 1){
        domain->getRenderer()->deserialize(argv[1]);
        return Nebulite::Constants::ErrorTable::NONE();
    }
    else{
        // no name provided, load empty env
        domain->getRenderer()->deserialize("{}");
        return Nebulite::Constants::ErrorTable::NONE();
    }
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::env_deload(int argc, char* argv[]){
    domain->getRenderer()->purgeObjects();
    domain->getRenderer()->purgeTextures();
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::spawn(int argc, char* argv[]){
    if(argc>1){
        // Using all args, allowing for whitespaces in the link and in the following functioncalls:
        // e.g.: spawn Planets/sun.jsonc|set text.str This is a sun
        std::string linkOrObject = argv[1];
        for (int i = 2; i < argc; i++) {
            linkOrObject += " " + std::string(argv[i]);
        }

        // Check if the file exists
        std::string link = Nebulite::Utility::StringHandler::untilSpecialChar(linkOrObject,'|');
        if(!Nebulite::Utility::FileManagement::fileExists(link)){
            // Check in standard directories
            static std::vector<std::string> standardDirectories = {
                "./Resources/Renderobjects/",
                "./Resources/RenderObjects/"
            };

            // Check all standard directories for the file
            bool found = false;
            for(const auto& prefix : standardDirectories){
                std::string testLink = prefix + link;
                if(Nebulite::Utility::FileManagement::fileExists(testLink)){
                    linkOrObject = prefix + linkOrObject;
                    found = true;
                    break;
                }
            }

            // Not found in standard directories either
            if(!found){
                return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
            }
        }

        // Create object with link to globalspace
        Nebulite::Core::RenderObject* ro = new Nebulite::Core::RenderObject(domain);
        ro->deserialize(linkOrObject);

        // Append to renderer
        // Renderer manages the RenderObjects lifetime
        domain->getRenderer()->append(ro);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::setResolution(int argc, char* argv[]){
    int w,h,scalar;
    w = 1000;
    h = 1000;
    scalar = 1;
    if(argc > 1){
        w = std::stoi(argv[1]);
    }
    if(argc > 2){
        h = std::stoi(argv[2]);
    }
    if(argc > 3){
        scalar = std::stoi(argv[3]);
    }
    domain->getRenderer()->changeWindowSize(w,h,scalar);
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::setFPS(int argc, char* argv[]){
    int fps = 60;
    if(argc == 2){
        fps = std::stoi(argv[1]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        
    }
    domain->getRenderer()->setTargetFPS(fps);
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::showFPS(int argc, char* argv[]){
    if(argc < 2){
        domain->getRenderer()->toggleFps(true);
    }
    else{
        if(!strcmp(argv[1], "on")){
            domain->getRenderer()->toggleFps(true);
        }
        else if(!strcmp(argv[1], "off")){
            domain->getRenderer()->toggleFps(false);
        }
        else{
            // unknown arg
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::cam_move(int argc, char* argv[]){
    if (argc < 3) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 3) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    int dx = std::stoi(argv[1]);
    int dy = std::stoi(argv[2]);
    domain->getRenderer()->moveCam(dx,dy);
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::cam_set(int argc, char* argv[]){
    if(argc == 3){
        int x = std::stoi(argv[1]);
        int y = std::stoi(argv[2]);
        domain->getRenderer()->setCam(x,y);
        return Nebulite::Constants::ErrorTable::NONE();
    }
    if(argc == 4){
        if(!strcmp(argv[3], "c")){
            int x = std::stoi(argv[1]);
            int y = std::stoi(argv[2]);
            domain->getRenderer()->setCam(x,y,true);
            return Nebulite::Constants::ErrorTable::NONE();
        }
        else{
            // unknown arg
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }
    else if(argc > 4){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::snapshot(int argc, char* argv[]){
    if(argc == 1){
        // No link provided, use default
        bool success = domain->getRenderer()->snapshot("./Resources/Snapshots/snapshot.png");
        if (!success) {
            return Nebulite::Constants::ErrorTable::RENDERER::CRITICAL_RENDERER_SNAPSHOT_FAILED();
        }
        return Nebulite::Constants::ErrorTable::NONE();
    }
    else if(argc == 2){
        // Link provided
        std::string link = argv[1];
        bool success = domain->getRenderer()->snapshot(link);
        if (!success) {
            return Nebulite::Constants::ErrorTable::RENDERER::CRITICAL_RENDERER_SNAPSHOT_FAILED();
        }
        return Nebulite::Constants::ErrorTable::NONE();
    }
    else{
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::beep(int argc, char* argv[]){
    // Beep function for debugging, from SDL
    domain->getRenderer()->beep();
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::selectedObject_get(int argc, char* argv[]) {
    if (argc != 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    uint32_t id = std::stoi(argv[1]);
    Nebulite::Core::RenderObject* obj = domain->getRenderer()->getObjectFromId(id);
    
    if (obj) {
        selectedRenderObject = obj;
        return Nebulite::Constants::ErrorTable::NONE();
    } else {
        selectedRenderObject = nullptr;
        return Nebulite::Constants::ErrorTable::addError("No RenderObject with the specified ID found.", Nebulite::Constants::Error::NON_CRITICAL);
    }
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Renderer::selectedObject_Parse(int argc, char* argv[]) {
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string command;
    for (int i = 1; i < argc; ++i) {    // Ignoring first 2 argc: <from> <thisFunctionsName>
        command += argv[i];
        if (i < argc - 1) command += " ";
    }
    if(selectedRenderObject == nullptr){
        return Nebulite::Constants::ErrorTable::addError("No RenderObject selected! Use selectedObject_get <id> to select a valid object.", Nebulite::Constants::Error::NON_CRITICAL);
    }

    return selectedRenderObject->parseStr(std::string(__FUNCTION__) + " " + command);
}