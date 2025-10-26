/**
 * @file KeyNames.hpp
 * @brief Contains all hardcoded key names used within Nebulite.
 * 
 * @todo Move key names into classes as static members!
 */
#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

//------------------------------------------
// Includes

// General
#include <string>

//------------------------------------------
namespace Nebulite::Constants {
/**
 * @struct keyName
 * @brief Contains all hardcoded key names used within Nebulite.
 */
const struct keyName {
    /**
     * @struct renderer
     * @brief Contains all hardcoded key names used within the Nebulite::Core::Renderer class.
     */
    const struct renderer{
        std::string _self = "renderer";
        std::string dispResX = "display.resolution.X";
        std::string dispResY = "display.resolution.Y";
        std::string positionX = "display.position.X";
        std::string positionY = "display.position.Y";
        std::string time_t = "time.t";
        std::string time_t_ms = "time.t_ms";
        std::string time_dt = "time.dt";
        std::string time_dt_ms = "time.dt_ms";
    }renderer;

    /**
     * @struct Random
     * @brief Contains all hardcoded key names used for random number generation.
     */
    const struct RNGs{
        std::string A = "random.A";
        std::string B = "random.B";
        std::string C = "random.C";
        std::string D = "random.D";
        std::string min = "random.min";
        std::string max = "random.max";
    }RNGs;

    /**
     * @struct renderObject
     * @brief Contains all hardcoded key names used within the Nebulite::Core::RenderObject class.
     */
    const struct renderObject{
        std::string _self = "renderObject";
        std::string id = "id";
        std::string positionX = "posX";
        std::string positionY = "posY";
        std::string layer = "layer";
        std::string pixelSizeX = "sprite.sizeX";
        std::string pixelSizeY = "sprite.sizeY";
        std::string imageLocation = "sprite.link";
        std::string isSpritesheet = "sprite.spritesheet.isSpritesheet";
        std::string spritesheetSizeX = "sprite.spritesheet.sizeX";
        std::string spritesheetSizeY = "sprite.spritesheet.sizeY";
        std::string spritesheetOffsetX = "sprite.spritesheet.offsetX";
        std::string spritesheetOffsetY = "sprite.spritesheet.offsetY";
        std::string textFontsize = "text.fontSize";
        std::string textStr = "text.str";
        std::string textDx = "text.dx";
        std::string textDy = "text.dy";
        std::string textColorR = "text.color.R";
        std::string textColorG = "text.color.G";
        std::string textColorB = "text.color.B";
        std::string textColorA = "text.color.A";
        std::string invokes = "invokes";
        std::string invokeSubscriptions = "invokeSubscriptions";
    }renderObject;

    /**
     * @struct invoke
     * @brief Contains all hardcoded key names used within the Nebulite::Interaction::Invoke class.
     */
    const struct invoke{
        std::string _self = "invoke";
        std::string logicalArg = "logicalArg";
        std::string functioncalls_global = "functioncalls_global";
        std::string functioncalls_self = "functioncalls_self";
        std::string functioncalls_other = "functioncalls_other";
        std::string topic = "topic";
        std::string exprVector = "exprs";
        std::string typeSelf = "self";
        std::string typeOther = "other";
        std::string typeGlobal = "global";
    }invoke;

    /**
     * @struct Environment
     * @brief Contains all hardcoded key names used within the Nebulite::Core::Environment class.
     */
    const struct Environment {
        std::string renderObjectContainer = "RenderObjectContainer";
    }environment;
}keyName;
}   // namespace Nebulite::Constants
#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP