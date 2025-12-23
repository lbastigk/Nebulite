/**
 * @file KeyNames.hpp
 * @brief Defines constant key names used throughout the Nebulite framework.
 */

#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

#include <string_view>

namespace Nebulite::Constants {

struct KeyNames {
    struct Renderer {
        inline static constexpr std::string_view self = "renderer";
        inline static constexpr std::string_view dispResX = "display.resolution.X";
        inline static constexpr std::string_view dispResY = "display.resolution.Y";
        inline static constexpr std::string_view positionX = "display.position.X";
        inline static constexpr std::string_view positionY = "display.position.Y";
        inline static constexpr std::string_view time_t = "time.t";
        inline static constexpr std::string_view time_t_ms = "time.t_ms";
        inline static constexpr std::string_view time_dt = "time.dt";
        inline static constexpr std::string_view time_dt_ms = "time.dt_ms";
    };

    struct RNGs {
        inline static constexpr std::string_view A = "random.A";
        inline static constexpr std::string_view B = "random.B";
        inline static constexpr std::string_view C = "random.C";
        inline static constexpr std::string_view D = "random.D";
        inline static constexpr std::string_view min = "random.min";
        inline static constexpr std::string_view max = "random.max";
    };

    struct RenderObject {
        inline static constexpr std::string_view self = "renderObject";
        inline static constexpr std::string_view id = "id";
        inline static constexpr std::string_view positionX = "posX";
        inline static constexpr std::string_view positionY = "posY";
        inline static constexpr std::string_view layer = "layer";

        inline static constexpr std::string_view pixelSizeX = "sprite.sizeX";
        inline static constexpr std::string_view pixelSizeY = "sprite.sizeY";
        inline static constexpr std::string_view imageLocation = "sprite.link";
        inline static constexpr std::string_view isSpritesheet = "sprite.spritesheet.isSpritesheet";
        inline static constexpr std::string_view spritesheetSizeX = "sprite.spritesheet.sizeX";
        inline static constexpr std::string_view spritesheetSizeY = "sprite.spritesheet.sizeY";
        inline static constexpr std::string_view spritesheetOffsetX = "sprite.spritesheet.offsetX";
        inline static constexpr std::string_view spritesheetOffsetY = "sprite.spritesheet.offsetY";

        inline static constexpr std::string_view invokes = "invokes";
        inline static constexpr std::string_view invokeSubscriptions = "invokeSubscriptions";

        inline static constexpr std::string_view textFontsize = "text.fontSize";
        inline static constexpr std::string_view textStr = "text.str";
        inline static constexpr std::string_view textColorR = "text.color.R";
        inline static constexpr std::string_view textColorG = "text.color.G";
        inline static constexpr std::string_view textColorB = "text.color.B";
        inline static constexpr std::string_view textColorA = "text.color.A";
        inline static constexpr std::string_view textDx = "text.dx";
        inline static constexpr std::string_view textDy = "text.dy";
    };

    struct Invoke {
        inline static constexpr std::string_view self = "invoke";
        inline static constexpr std::string_view typeSelf = "self";
        inline static constexpr std::string_view typeOther = "other";
        inline static constexpr std::string_view typeGlobal = "global";

        inline static constexpr std::string_view logicalArg = "logicalArg";
        inline static constexpr std::string_view topic = "topic";
        inline static constexpr std::string_view exprVector = "exprs";
        inline static constexpr std::string_view functioncalls_global = "functioncalls.global";
        inline static constexpr std::string_view functioncalls_self = "functioncalls.self";
        inline static constexpr std::string_view functioncalls_other = "functioncalls.other";
    };

    struct Ruleset {
        inline static constexpr std::string_view self = "ruleset";
        inline static constexpr std::string_view topic = "topic";
        inline static constexpr std::string_view condition = "condition";
        inline static constexpr std::string_view assignments = "assignments";
        inline static constexpr std::string_view parseOnGlobal = "functioncalls.global";
        inline static constexpr std::string_view parseOnSelf   = "functioncalls.self";
        inline static constexpr std::string_view parseOnOther  = "functioncalls.other";
    };

    struct Environment {
        inline static constexpr std::string_view renderObjectContainer = "RenderObjectContainer";
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
