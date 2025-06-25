#pragma once

#include <string>

const struct keyName {
    const struct renderObject{
        std::string _self = "renderObject";
        std::string id = "id";
        std::string positionX = "posX";
        std::string positionY = "posY";
        std::string layer = "layer";
        std::string deleteFlag = "deleteFlag";
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
        std::string flagCalculate = "text.flagCalculate";
        std::string invokes = "invokes";
        std::string invokeSubscriptions = "invokeSubscriptions";
        std::string reloadInvokes = "invokeReload";
    }renderObject;
    const struct MyTemplate {
        std::string _self = "MyTemplate";
        std::string bsp1 = "Beispiel1";
    }MyTemplate;
    const struct Environment {
        std::string renderObjectContainer = "RenderObjectContainer";
    }environment;
    const struct testImages {
        std::string folder001 = "Resources/Sprites/TEST001P/";
        std::string folder100 = "Resources/Sprites/TEST100P/";
    }testImages;
}keyName;
