#include "GTE_Renderer.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::envload(int argc, char* argv[]){
    if(argc > 1){
        self->getRenderer()->deserialize(argv[1]);
        return Nebulite::ERROR_TYPE::NONE;
    }
    else{
        // no name provided, load empty env
        self->getRenderer()->deserialize("{}");
        return Nebulite::ERROR_TYPE::NONE;
    }
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::envdeload(int argc, char* argv[]){
    self->getRenderer()->purgeObjects();
    self->getRenderer()->purgeTextures();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::spawn(int argc, char* argv[]){
    if(argc>1){
        // Using all args, allowing for whitespaces in the link and in the following functioncalls:
        // e.g.: spawn Planets/sun.json|set text.str This is a sun
        std::string linkOrObject = argv[1];
        for (int i = 2; i < argc; i++) {
            linkOrObject += " " + std::string(argv[i]);
        }

        // [TODO] Add standard-directories to find files in:
        // spawn Planets/sun.json -> spawn ./Resources/Renderobjects/Planets/sun.json
        // Note that the link cant be turned into a serial here, 
        // due to additional passings like |posX=100
        // that are resolved in Renderobject::deserialize / JSON::deserialize

        // Create object
        RenderObject* ro = new RenderObject;
        ro->deserialize(linkOrObject);

        // Append to renderer
        // Renderer manages the RenderObjects lifetime
        self->getRenderer()->append(ro);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::setResolution(int argc, char* argv[]){
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
    self->getRenderer()->changeWindowSize(w,h,scalar);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::setFPS(int argc, char* argv[]){
    if(argc != 2){
        self->getRenderer()->setFPS(60);
    }
    else{
        int fps = std::stoi(argv[1]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        self->getRenderer()->setFPS(fps);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::moveCam(int argc, char* argv[]){
    if (argc < 3) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    int dx = std::stoi(argv[1]);
    int dy = std::stoi(argv[2]);
    self->getRenderer()->moveCam(dx,dy);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::setCam(int argc, char* argv[]){
    if(argc == 3){
        int x = std::stoi(argv[1]);
        int y = std::stoi(argv[2]);
        self->getRenderer()->setCam(x,y);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc == 4){
        if(!strcmp(argv[3], "c")){
            int x = std::stoi(argv[1]);
            int y = std::stoi(argv[2]);
            self->getRenderer()->setCam(x,y,true);
            return Nebulite::ERROR_TYPE::NONE;
        }
        else{
            // unknown arg
            return Nebulite::ERROR_TYPE::UNKNOWN_ARG;
        }
    }
    else if(argc > 4){
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::snapshot(int argc, char* argv[]){
    if(argc == 1){
        // No link provided, use default
        bool success = self->getRenderer()->snapshot();
        if (!success) {
            return Nebulite::ERROR_TYPE::SNAPSHOT_FAILED;
        }
        return Nebulite::ERROR_TYPE::NONE;
    }
    else if(argc == 2){
        // Link provided
        std::string link = argv[1];
        bool success = self->getRenderer()->snapshot(link);
        if (!success) {
            return Nebulite::ERROR_TYPE::SNAPSHOT_FAILED;
        }
        return Nebulite::ERROR_TYPE::NONE;
    }
    else{
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::beep(int argc, char* argv[]){
    // Beep function for debugging, from SDL
    self->getRenderer()->beep();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::getObjectFromId(int argc, char* argv[]) {
    if (argc != 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }

    uint32_t id = std::stoi(argv[1]);
    Nebulite::RenderObject* obj = self->getRenderer()->getObjectFromId(id);
    
    if (obj) {
        self->selectedRenderObject = obj;
        return Nebulite::ERROR_TYPE::NONE;
    } else {
        self->selectedRenderObject = nullptr;
        // Not seen as an error, just no object found
        return Nebulite::ERROR_TYPE::NONE;
    }
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::Renderer::printSelectedObject(int argc, char* argv[]) {
    if (self->selectedRenderObject) {
        std::cout << "Selected Renderobject: \n" << self->selectedRenderObject->serialize() << std::endl;
        return Nebulite::ERROR_TYPE::NONE;
    } else {
        std::cout << "No renderobject selected." << std::endl;
        return Nebulite::ERROR_TYPE::NONE;
    }
}