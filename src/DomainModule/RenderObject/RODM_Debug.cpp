#include "DomainModule/RenderObject/RODM_Debug.hpp"

#include "Core/RenderObject.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::RenderObject {

std::string const Debug::debug_name = "debug";
std::string const Debug::debug_desc = R"(Debugging functions for RenderObject)";

//------------------------------------------
// Update

Constants::Error Debug::update(){
    // For on-tick-updates
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

// NOLINTNEXTLINE
Constants::Error Debug::eval(int argc,  char** argv){
    std::string const args = Utility::StringHandler::recombineArgs(argc, argv);
    std::string const evaluatedArgs = domain->getGlobalSpace()->eval(args, domain);
    return domain->parseStr(evaluatedArgs);
}
std::string const Debug::eval_name = "eval";
std::string const Debug::eval_desc = R"(Evaluate an expression and execute the result. 
Example: eval echo $(1+1)

Examples:
     
eval echo $(1+1)    outputs:    2.000000
eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
)";

// NOLINTNEXTLINE
Constants::Error Debug::printSrcRect(int argc,  char** argv){
    if(argc != 1){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    if(SDL_Rect const* srcRect = domain->getSrcRect();srcRect){
        std::string message;
        message += "Source Rectangle:";
        message += "{ x: " + std::to_string(srcRect->x);
        message += ", y: " + std::to_string(srcRect->y);
        message += ", w: " + std::to_string(srcRect->w);
        message += ", h: " + std::to_string(srcRect->h);
        message += " }";
        logln(message);
    } else {
        logln("This RenderObject is not a spritesheet.");
    }

    return Constants::ErrorTable::NONE();
}
std::string const Debug::printSrcRect_name = "debug print-src-rect";
std::string const Debug::printSrcRect_desc = R"(Prints debug information about the source rectangle to console

Usage: debug print-src-rect

Outputs:
Source Rectangle: { x: ..., y: ..., w: ..., h: ... }
If the RenderObject is not a spritesheet, indicates that instead:
This RenderObject is not a spritesheet.
)";

// NOLINTNEXTLINE
Constants::Error Debug::printDstRect(int argc,  char** argv){
    if(argc != 1){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }


    if(SDL_Rect const* dstRect = domain->getDstRect(); dstRect){
        std::string message;
        message += "Destination Rectangle:";
        message += "{ x: " + std::to_string(dstRect->x);
        message += ", y: " + std::to_string(dstRect->y);
        message += ", w: " + std::to_string(dstRect->w);
        message += ", h: " + std::to_string(dstRect->h);
        message += " }";
        logln(message);
    } else {
        logln("Destination rectangle is not set.");
    }

    return Constants::ErrorTable::NONE();
}
std::string const Debug::printDstRect_name = "debug print-dst-rect";
std::string const Debug::printDstRect_desc = R"(Prints debug information about the destination rectangle to console

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
    std::string getTextureAccessString(int const& accessType){
        return accessType == SDL_TEXTUREACCESS_STATIC    ? "Static"    :
               accessType == SDL_TEXTUREACCESS_STREAMING ? "Streaming" :
               accessType == SDL_TEXTUREACCESS_TARGET    ? "Target"    :
               "Other";
    }

    /**
     * @brief Converts SDL pixel format enum to human-readable string.
     * @param format The SDL pixel format enum value.
     * @return A string representing the pixel format.
     */
    std::string getTextureFormatString(Uint32 const& format){
        return format == SDL_PIXELFORMAT_RGBA8888    ? "RGBA8888"  :
               format == SDL_PIXELFORMAT_ARGB8888    ? "ARGB8888"  :
               format == SDL_PIXELFORMAT_RGB888      ? "RGB888"    :
               format == SDL_PIXELFORMAT_BGR888      ? "BGR888"    :
               format == SDL_PIXELFORMAT_RGB565      ? "RGB565"    :
               format == SDL_PIXELFORMAT_RGB555      ? "RGB555"    :
               format == SDL_PIXELFORMAT_ARGB1555    ? "ARGB1555"  :
               format == SDL_PIXELFORMAT_ABGR8888    ? "ABGR8888"  :
               format == SDL_PIXELFORMAT_BGRA8888    ? "BGRA8888"  :
               "Other";
    }

    /**
     * @brief Prints detailed information about an SDL_Texture.
     * 
     * @param texture Pointer to the SDL_Texture to query.
     */
    std::string getTextureInfoString(SDL_Texture* texture){
        std::string info;
        if(texture){
            Uint32 format;
            int accessType, w, h;
            if (SDL_QueryTexture(texture, &format, &accessType, &w, &h) == 0){
                // Decode format and access to human-readable strings
                std::string const accessStr = getTextureAccessString(accessType);
                std::string const formatStr = getTextureFormatString(format);

                // Print texture details
                info += " - Width  : " + std::to_string(w) + "\n";
                info += " - Height : " + std::to_string(h) + "\n";
                info += " - Access : " + accessStr + "\n";
                info += " - Format : " + formatStr + "\n";
            } else {
                 info += "Failed to query texture: " + std::string(SDL_GetError());
            }
        } else {
            info += "No texture is associated with this RenderObject.";
        }
        return info;
    }
}

// NOLINTNEXTLINE
Constants::Error Debug::textureStatus(int argc,  char** argv){
    if(argc != 1){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS(); // No arguments expected
    }

    //------------------------------------------
    // Print Texture Status
    logln("Texture Status:");

    // Nebulite info
    logln(std::string(" - Texture Key   : ") + domain->get<std::string>(Constants::keyName.renderObject.imageLocation.c_str(), "None"));
    logln(std::string(" - Valid Texture : ") + (domain->getTexture()->isTextureValid() ? "Yes" : "No"));
    logln(std::string(" - Local Texture : ") + (domain->getTexture()->isTextureStoredLocally() ? "Yes" : "No"));

    // SDL info
    logln("SDL Texture Info:");
    SDL_Texture* texture = domain->getTexture()->getSDLTexture();   // Going the long way to ensure outside access is validated
    logln(getTextureInfoString(texture));
    return Constants::ErrorTable::NONE();
}
std::string const Debug::textureStatus_name = "debug texture-status";
std::string const Debug::textureStatus_desc = R"(Prints debug information about the texture to console

Usage: debug texture-status

Outputs various details about the texture, including:
 - Texture Key
 - Valid Texture
 - Local Texture
 - SDL Texture Info (Width, Height, Access, Format)
)";

} // namespace Nebulite::DomainModule::RenderObject