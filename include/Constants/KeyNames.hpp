/**
 * @file KeyNames.hpp
 * @brief Defines constant key names used throughout the Nebulite framework.
 * @details While for DomainModules we can write scoped keys directly within their classes,
 *          this is not feasible for Domain-Related keys due to their hierarchical nature.
 *          Specifically, DomainModules are never dependent on other DomainModules besides their key names.
 *          But Domains can own other Domains, requiring knowledge of their key names.
 *          Therefore, we centralize these key names here to avoid circular dependencies
 *          and to ensure consistency across the framework.
 * @note Work in progress, many terribly organized, duplicated, or unused keys exist here.
 *       This file will be refactored over time to improve organization and safety.
 * @todo Sort keys into their respective scopes and Domains
 * @todo Refactor DomainModule keys into DomainModules instead of having them here
 * @todo Turn all into Data::ScopedKey
 * @todo Add scopes where applicable to improve safety
 */

#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

#include <string_view>
#include "Data/Document/JsonScope.hpp"

namespace Nebulite::Constants {

// TODO: Use scopedKey instead of string_view for better scope safety
// TODO: Remove unused keys and refactor used ones

// Example usage:
static constexpr Data::ScopedKey testKey{"renderer.", "time.t"}; // Only JsonScopes with scopePrefix "renderer." can use this key

struct KeyNames {
    struct Renderer {
        static constexpr Data::ScopedKey dispResX{"renderer.","resolution.X"};
        static constexpr Data::ScopedKey dispResY{"renderer.","resolution.Y"};
        static constexpr Data::ScopedKey positionX{"renderer.","position.X"};
        static constexpr Data::ScopedKey positionY{"renderer.","position.Y"};
    };

    struct GlobalSpace {
        // TODO: Unify with DomainModule::GlobalSpace::Time
        //       Currently duplicated keys here and in that module
        static constexpr Data::ScopedKey time_t{"","time.t"};
        static constexpr Data::ScopedKey time_t_ms{"","time.t_ms"};
        static constexpr Data::ScopedKey time_dt{"","time.dt"};
        static constexpr Data::ScopedKey time_dt_ms{"","time.dt_ms"};

        // Keys for RNG
        struct RNG {
            static constexpr Data::ScopedKey A{"random.","A"};
            static constexpr Data::ScopedKey B{"random.","B"};
            static constexpr Data::ScopedKey C{"random.","C"};
            static constexpr Data::ScopedKey D{"random.","D"};
            static constexpr Data::ScopedKey min{"random.","min"};
            static constexpr Data::ScopedKey max{"random.","max"};
        };
    };

    // TODO: Part of GlobalSpace?
    struct RNGs {
        // TODO: Use "random." as scope, and modify keys to be relative to that scope
        //static constexpr std::string_view A = "random.A";
        //static constexpr std::string_view B = "random.B";
        //static constexpr std::string_view C = "random.C";
        //static constexpr std::string_view D = "random.D";
        //static constexpr std::string_view min = "random.min";
        //static constexpr std::string_view max = "random.max";
    };

    struct RenderObject {
        // TODO: Use "" as scope, and modify keys to be relative to that scope
        //static constexpr std::string_view self = "renderObject";
        static constexpr std::string_view id = "id";
        static constexpr std::string_view positionX = "posX";
        static constexpr std::string_view positionY = "posY";
        static constexpr std::string_view layer = "layer";

        // TODO: Use "texture." as scope
        static constexpr std::string_view pixelSizeX = "sprite.sizeX";
        static constexpr std::string_view pixelSizeY = "sprite.sizeY";
        static constexpr std::string_view imageLocation = "sprite.link";
        static constexpr std::string_view isSpritesheet = "sprite.spritesheet.isSpritesheet";
        static constexpr std::string_view spritesheetSizeX = "sprite.spritesheet.sizeX";
        static constexpr std::string_view spritesheetSizeY = "sprite.spritesheet.sizeY";
        static constexpr std::string_view spritesheetOffsetX = "sprite.spritesheet.offsetX";
        static constexpr std::string_view spritesheetOffsetY = "sprite.spritesheet.offsetY";
        static constexpr std::string_view textFontsize = "text.fontSize";
        static constexpr std::string_view textStr = "text.str";
        static constexpr std::string_view textColorR = "text.color.R";
        static constexpr std::string_view textColorG = "text.color.G";
        static constexpr std::string_view textColorB = "text.color.B";
        static constexpr std::string_view textColorA = "text.color.A";
        static constexpr std::string_view textDx = "text.dx";
        static constexpr std::string_view textDy = "text.dy";

        // TODO: Are these still used?
        static constexpr std::string_view invokes = "invokes";
        static constexpr std::string_view invokeSubscriptions = "invokeSubscriptions";
    };

    struct Invoke {
        // TODO: Are these still used?
        static constexpr std::string_view self = "invoke";
        static constexpr std::string_view typeSelf = "self";
        static constexpr std::string_view typeOther = "other";
        static constexpr std::string_view typeGlobal = "global";

        static constexpr std::string_view logicalArg = "logicalArg";
        static constexpr std::string_view topic = "topic";
        static constexpr std::string_view exprVector = "exprs";
        static constexpr std::string_view functioncalls_global = "functioncalls.global";
        static constexpr std::string_view functioncalls_self = "functioncalls.self";
        static constexpr std::string_view functioncalls_other = "functioncalls.other";
    };

    // TODO: Move to DomainModule for RenderObject Ruleset handling
    struct Ruleset {
        static constexpr std::string_view self = "ruleset";
        static constexpr std::string_view topic = "topic";
        static constexpr std::string_view condition = "condition";
        static constexpr std::string_view assignments = "assignments";
        static constexpr std::string_view parseOnGlobal = "functioncalls.global";
        static constexpr std::string_view parseOnSelf   = "functioncalls.self";
        static constexpr std::string_view parseOnOther  = "functioncalls.other";
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
