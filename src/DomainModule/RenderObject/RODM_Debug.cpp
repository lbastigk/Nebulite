#include "DomainModule/RenderObject/RODM_Debug.hpp"

#include "Core/RenderObject.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::RenderObject {

const std::string Debug::debug_name = "debug";
const std::string Debug::debug_desc = R"(Debugging functions for RenderObject)";

//------------------------------------------
// Update
Nebulite::Constants::Error Debug::update() {
    // For on-tick-updates
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Nebulite::Constants::Error Debug::eval(int argc,  char* argv[]) {
    // argc/argv to string for evaluation
    std::string args = Nebulite::Utility::StringHandler::recombineArgs(argc, argv);

    // Evaulate with context of this RenderObject
    std::string args_evaled = domain->getGlobalSpace()->eval(args, domain);

    // reparse
    return domain->parseStr(args_evaled);
}
const std::string Debug::eval_name = "eval";
const std::string Debug::eval_desc = R"(Evaluate an expression and execute the result. 
Example: eval echo $(1+1)

Examples:
     
eval echo $(1+1)    outputs:    2.000000
eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
)";

Nebulite::Constants::Error Debug::printSrcRect(int argc,  char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    const SDL_Rect* srcRect = domain->getSrcRect();
    if(srcRect) {
        std::cout << "Source Rectangle: { x: " << srcRect->x << ", y: " << srcRect->y 
                  << ", w: " << srcRect->w << ", h: " << srcRect->h << " }" << std::endl;
    } else {
        std::cout << "This RenderObject is not a spritesheet." << std::endl;
    }

    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::printSrcRect_name = "debug print-src-rect";
const std::string Debug::printSrcRect_desc = R"(Prints debug information about the source rectangle to console

Usage: debug print-src-rect

Outputs:
Source Rectangle: { x: ..., y: ..., w: ..., h: ... }
If the RenderObject is not a spritesheet, indicates that instead:
This RenderObject is not a spritesheet.
)";

Nebulite::Constants::Error Debug::printDstRect(int argc,  char* argv[]) {
    if(argc != 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    const SDL_Rect* dstRect = domain->getDstRect();
    if(dstRect) {
        std::cout << "Destination Rectangle: { x: " << dstRect->x << ", y: " << dstRect->y 
                  << ", w: " << dstRect->w << ", h: " << dstRect->h << " }" << std::endl;
    } else {
        std::cout << "Destination rectangle is not set." << std::endl;
    }

    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::printDstRect_name = "debug print-dst-rect";
const std::string Debug::printDstRect_desc = R"(Prints debug information about the destination rectangle to console

Usage: debug print-dst-rect

Outputs:
Destination Rectangle: { x: ..., y: ..., w: ..., h: ... }
If the RenderObject is not a spritesheet, indicates that instead:
Destination rectangle is not set.
)";

// Texture debugging helper
namespace{

    /**
     * @brief Converts SDL texture access enum to human-readable string.
     * @param accessType The SDL texture access enum value.
     * @return A string representing the access type.
     */
    std::string getTextureAccessString(int accessType) {
        return (accessType == SDL_TEXTUREACCESS_STATIC)    ? "Static"    :
               (accessType == SDL_TEXTUREACCESS_STREAMING) ? "Streaming" :
               (accessType == SDL_TEXTUREACCESS_TARGET)    ? "Target"    :
               "Other";
    }

    /**
     * @brief Converts SDL pixel format enum to human-readable string.
     * @param format The SDL pixel format enum value.
     * @return A string representing the pixel format.
     */
    std::string getTextureFormatString(Uint32 format) {
        return (format == SDL_PIXELFORMAT_RGBA8888)    ? "RGBA8888"  :
               (format == SDL_PIXELFORMAT_ARGB8888)    ? "ARGB8888"  :
               (format == SDL_PIXELFORMAT_RGB888)      ? "RGB888"    :
               (format == SDL_PIXELFORMAT_BGR888)      ? "BGR888"    :
               (format == SDL_PIXELFORMAT_RGB565)      ? "RGB565"    :
               (format == SDL_PIXELFORMAT_RGB555)      ? "RGB555"    :
               (format == SDL_PIXELFORMAT_ARGB1555)    ? "ARGB1555"  :
               (format == SDL_PIXELFORMAT_ABGR8888)    ? "ABGR8888"  :
               (format == SDL_PIXELFORMAT_BGRA8888)    ? "BGRA8888"  :
               "Other";
    }

    /**
     * @brief Prints detailed information about an SDL_Texture.
     * 
     * @param texture Pointer to the SDL_Texture to query.
     */
    void printTextureInfo(SDL_Texture* texture) {
        if(texture) {
            Uint32 format;
            int accessType, w, h;
            if (SDL_QueryTexture(texture, &format, &accessType, &w, &h) == 0) {
                // Decode format and access to human-readable strings
                std::string accessStr = getTextureAccessString(accessType);
                std::string formatStr = getTextureFormatString(format);

                // Print texture details
                std::cout << " - Width  : " << w << std::endl;
                std::cout << " - Height : " << h << std::endl;
                std::cout << " - Access : " << accessStr << std::endl;
                std::cout << " - Format : " << formatStr << std::endl;
            } else {
                std::cerr << "Failed to query texture: " << SDL_GetError() << std::endl;
            }
        } else {
            std::cout << "No texture is associated with this RenderObject." << std::endl;
        }
    }
}

Nebulite::Constants::Error Debug::textureStatus(int argc,  char* argv[]){
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
    printTextureInfo(texture);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::textureStatus_name = "debug texture-status";
const std::string Debug::textureStatus_desc = R"(Prints debug information about the texture to console

Usage: debug texture-status

Outputs various details about the texture, including:
 - Texture Key
 - Valid Texture
 - Local Texture
 - SDL Texture Info (Width, Height, Access, Format)
)";

} // namespace Nebulite::DomainModule::RenderObject