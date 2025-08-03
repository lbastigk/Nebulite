#pragma once

/*
The following Struct keyName is used to store all hardcoded key names used within
Nebulite to manipulate JSON objects like RenderObjects, Invokes, etc.
This allows to change the key names in one place only, if needed.

*/

#include <string>

namespace Nebulite{
const struct keyName {
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
        //std::string flagCalculate = "text.flagCalculate";
        std::string invokes = "invokes";
        std::string invokeSubscriptions = "invokeSubscriptions";
        std::string reloadInvokes = "invokeReload";
        std::string invokeVector = "invokes";
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
    const struct invokeTriple{
        std::string _self = "invokeTriple";
        std::string changeType = "changeType";
        std::string key = "key";
        std::string value = "value";
    }invokeTriple;
    const struct Environment {
        std::string renderObjectContainer = "RenderObjectContainer";
    }environment;
}keyName;
}