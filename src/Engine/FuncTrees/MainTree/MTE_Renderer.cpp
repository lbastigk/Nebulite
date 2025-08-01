#include "MTE_Renderer.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::envload(int argc, char* argv[]){
    if(argc > 1){
        global->getRenderer()->deserialize(argv[1]);
        return Nebulite::ERROR_TYPE::NONE;
    }
    else{
        // no name provided, load empty env
        global->getRenderer()->deserialize("{}");
        return Nebulite::ERROR_TYPE::NONE;
    }
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::envdeload(int argc, char* argv[]){
    global->getRenderer()->purgeObjects();
    global->getRenderer()->purgeTextures();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::spawn(int argc, char* argv[]){
    if(argc>1){
        std::string linkOrObject = argv[1];

        // Check if argv1 provided is an object
        if(linkOrObject.starts_with('{')){
            std::cerr << "Object-passing to spawn is currently not supported" << std::endl;
            return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
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
        global->getRenderer()->append(ro);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::setResolution(int argc, char* argv[]){
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
    global->getRenderer()->changeWindowSize(w,h,scalar);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::setFPS(int argc, char* argv[]){
    if(argc != 2){
        global->getRenderer()->setFPS(60);
    }
    else{
        int fps = std::stoi(argv[1]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        global->getRenderer()->setFPS(fps);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::moveCam(int argc, char* argv[]){
    if (argc < 3) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    int dx = std::stoi(argv[1]);
    int dy = std::stoi(argv[2]);
    global->getRenderer()->moveCam(dx,dy);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::setCam(int argc, char* argv[]){
    if(argc == 3){
        int x = std::stoi(argv[1]);
        int y = std::stoi(argv[2]);
        global->getRenderer()->setCam(x,y);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc == 4){
        if(!strcmp(argv[3], "c")){
            int x = std::stoi(argv[1]);
            int y = std::stoi(argv[2]);
            global->getRenderer()->setCam(x,y,true);
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

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::snapshot(int argc, char* argv[]){
    if(argc == 1){
        // No link provided, use default
        bool success = global->getRenderer()->snapshot();
        if (!success) {
            return Nebulite::ERROR_TYPE::SNAPSHOT_FAILED;
        }
        return Nebulite::ERROR_TYPE::NONE;
    }
    else if(argc == 2){
        // Link provided
        std::string link = argv[1];
        bool success = global->getRenderer()->snapshot(link);
        if (!success) {
            return Nebulite::ERROR_TYPE::SNAPSHOT_FAILED;
        }
        return Nebulite::ERROR_TYPE::NONE;
    }
    else{
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
}

Nebulite::ERROR_TYPE Nebulite::MainTreeExpansion::Renderer::beep(int argc, char* argv[]){
    // Beep function for debugging, from SDL
    global->getRenderer()->beep();
    return Nebulite::ERROR_TYPE::NONE;
}