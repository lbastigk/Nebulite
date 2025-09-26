#pragma once

/**
 * @file KeyNames.hpp
 * @brief Contains all hardcoded key names used within Nebulite.
 */

//------------------------------------------
// Includes

// General
#include <string>

//------------------------------------------
namespace Nebulite{
namespace Constants {
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
        std::string time_fixed_dt_ms = "time.fixed_dt_ms";
        std::string time_t = "time.t";
        std::string time_t_ms = "time.t_ms";
        std::string time_dt = "time.dt";
        std::string time_dt_ms = "time.dt_ms";
    }renderer;

    /**
     * @struct Random
     * @brief Contains all hardcoded key names used for random number generation.
     * 
     * @todo replace rand/rrand
     * @todo Proper implementation and usage within GDM_RNG, probably helpful to keep rand and rrand.
     * rrand is helpful, as it is incremented with each spawn, allowing for constant random value usage in a for-loop.
     * But maybe we can find a better way that rehashes on each functioncall. See GDM_RNG for ideas.
     */
    const struct Random{
        std::string A = "random.A";
        std::string B = "random.B";
        std::string C = "random.C";
        std::string D = "random.D";
        //std::string rand = "rand";
        //std::string rrand = "rrand";
    }random;

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
}   // namespace Constants
}   // namespace Nebulite