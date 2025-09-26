#include "DomainModule/RenderObject/RDM_Debug.hpp"

#include "Core/RenderObject.hpp"

void Nebulite::DomainModule::RenderObject::Debug::update() {
    // For on-tick-updates
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Debug::printSrcRect(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    SDL_Rect* srcRect = domain->getSrcRect();
    if(srcRect) {
        std::cout << "Source Rectangle: { x: " << srcRect->x << ", y: " << srcRect->y 
                  << ", w: " << srcRect->w << ", h: " << srcRect->h << " }" << std::endl;
    } else {
        std::cout << "This RenderObject is not a spritesheet." << std::endl;
    }

    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Debug::printDstRect(int argc, char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    SDL_Rect* dstRect = domain->getDstRect();
    if(dstRect) {
        std::cout << "Destination Rectangle: { x: " << dstRect->x << ", y: " << dstRect->y 
                  << ", w: " << dstRect->w << ", h: " << dstRect->h << " }" << std::endl;
    } else {
        std::cout << "Destination rectangle is not set." << std::endl;
    }

    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Debug::print(int argc, char* argv[]){
    std::string serialized = domain->serialize();
    std::cout << serialized << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Debug::printValue(int argc, char* argv[]){
    if(argc<2){
        std::cerr << "print-value requires a <key> argument" << std::endl;
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string key = argv[1];
    auto value = domain->get<std::string>(key.c_str(), "");
    std::cout << value << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::RenderObject::Debug::textureStatus(int argc, char* argv[]){
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    //------------------------------------------
    // Print Texture Status
    std::cout << "Texture Status:" << std::endl;

    // Nebulite info
    std::cout << " - Texture Key   : " << domain->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str(), "None") << std::endl;
    std::cout << " - Valid Texture : " << (domain->getTexture()->isTextureValid() ? "Yes" : "No") << std::endl;
    std::cout << " - Local Texture : " << (domain->getTexture()->isTextureStoredLocally() ? "Yes" : "No") << std::endl;

    // SDL info
    std::cout << "SDL Texture Info:" << std::endl;
    SDL_Texture* texture = domain->getTexture()->getSDLTexture();   // Going the long way to ensure outside access is validated
    if(texture) {
        Uint32 format;
        int access, w, h;
        if (SDL_QueryTexture(texture, &format, &access, &w, &h) == 0) {
            // Decode format and access to human-readable strings
            std::string accessStr = (access == SDL_TEXTUREACCESS_STATIC) ? "Static" :
                                    (access == SDL_TEXTUREACCESS_STREAMING) ? "Streaming" :
                                    (access == SDL_TEXTUREACCESS_TARGET) ? "Target" :
                                    "Other";
            std::string formatStr = (format == SDL_PIXELFORMAT_RGBA8888) ? "RGBA8888" :
                                    (format == SDL_PIXELFORMAT_ARGB8888) ? "ARGB8888" :
                                    (format == SDL_PIXELFORMAT_RGB888) ? "RGB888" :
                                    (format == SDL_PIXELFORMAT_BGR888) ? "BGR888" :
                                    (format == SDL_PIXELFORMAT_RGB565) ? "RGB565" :
                                    (format == SDL_PIXELFORMAT_RGB555) ? "RGB555" :
                                    (format == SDL_PIXELFORMAT_ARGB1555) ? "ARGB1555" :
                                    (format == SDL_PIXELFORMAT_ABGR8888) ? "ABGR8888" :
                                    (format == SDL_PIXELFORMAT_BGRA8888) ? "BGRA8888" :
                                    "Other";

            // Print texture details
            std::cout << " - Width         : " << w << std::endl;
            std::cout << " - Height        : " << h << std::endl;
            std::cout << " - Access        : " << accessStr << std::endl;
            std::cout << " - Format        : " << formatStr << std::endl;
        } else {
            std::cerr << "Failed to query texture: " << SDL_GetError() << std::endl;
            return Nebulite::Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_QUERY_FAILED();
        }
    } else {
        std::cout << "No texture is associated with this RenderObject." << std::endl;
    }

    // More info from Nebulite::Core::Texture directly:
    

    return Nebulite::Constants::ErrorTable::NONE();
}