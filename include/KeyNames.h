#pragma once

/*
The following Struct keyName is used to store all hardcoded key names used within
Nebulite to manipulate JSON objects like RenderObjects, Invokes, etc.
This allows to change the key names in one place only, if needed.

*/

#include <string>

namespace Nebulite{
const struct keyName {
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
    // TODO: replace rand/rrand with multiple randoms
    const struct Random{
        std::string A = "random.A";
        std::string B = "random.B";
        std::string C = "random.C";
        std::string D = "random.D";
    }random;
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
    const struct Environment {
        std::string renderObjectContainer = "RenderObjectContainer";
    }environment;
}keyName;
}   // namespace Nebulite