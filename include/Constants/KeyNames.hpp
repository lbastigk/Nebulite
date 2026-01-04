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
#include "Core/JsonScope.hpp"

namespace Nebulite::Constants {

// TODO: Use Data::ScopedKey::create<scope> instead of string_view for better scope safety
// TODO: Remove unused keys and refactor used ones
struct KeyNames {
    struct Renderer {
        static constexpr char scope[] = "renderer.";
        static constexpr auto dispResX = Data::ScopedKey::create<scope>("resolution.X");
        static constexpr auto dispResY = Data::ScopedKey::create<scope>("resolution.Y");
        static constexpr auto positionX = Data::ScopedKey::create<scope>("position.X");
        static constexpr auto positionY = Data::ScopedKey::create<scope>("position.Y");
    };

    struct GlobalSpace {
        static constexpr char scope[] = "";

        // TODO: Unify with DomainModule::GlobalSpace::Time
        //       Currently duplicated keys here and in that module
        static constexpr auto time_t = Data::ScopedKey::create<scope>("time.t");
        static constexpr auto time_t_ms = Data::ScopedKey::create<scope>("time.t_ms");
        static constexpr auto time_dt = Data::ScopedKey::create<scope>("time.dt");
        static constexpr auto time_dt_ms = Data::ScopedKey::create<scope>("time.dt_ms");

        // Keys for RNG
        // Due to the need of a pre-parse function in GlobalSpace (to modify RNGs before any command is parsed, making them deterministic),
        // these keys are defined here instead of in a DomainModule,
        // And the RNG implementation is directly in GlobalSpace as well
        struct RNG {
            static constexpr char scope[] = "random.";
            static constexpr auto A = Data::ScopedKey::create<scope>("A");
            static constexpr auto B = Data::ScopedKey::create<scope>("B");
            static constexpr auto C = Data::ScopedKey::create<scope>("C");
            static constexpr auto D = Data::ScopedKey::create<scope>("D");
            static constexpr auto min = Data::ScopedKey::create<scope>("min");
            static constexpr auto max = Data::ScopedKey::create<scope>("max");
        };

        // Keys for Physics
        // TODO: Set all to specific values in GlobalSpace
        //       Perhaps even in a DomainModule for GlobalSpace,
        //       Then we can move these keys there
        struct Physics {
            static constexpr char scope[] = "physics.";
            static constexpr auto G = Data::ScopedKey::create<scope>("G");
        };
    };

    struct RenderObject {
        // TODO: Use "" as scope, and modify keys to be relative to that scope
        //static constexpr std::string_view self = "renderObject";
        static constexpr std::string_view id = "id";
        static constexpr std::string_view positionX = "posX";
        static constexpr std::string_view positionY = "posY";
        static constexpr std::string_view layer = "layer";

        // Keys for Ruleset invocations and subscriptions
        struct Ruleset {
            static constexpr char scope[] = "ruleset.";
            static constexpr auto broadcast = Data::ScopedKey::create<scope>("broadcast");
            static constexpr auto listen = Data::ScopedKey::create<scope>("listen");
        };

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
    };

    // TODO: Integrate into RenderObject Ruleset DomainModule, remove unused keys
    struct Invoke {
        // TODO: Are these still used?
        static constexpr std::string_view self = "invoke";
        static constexpr std::string_view typeSelf = "self";
        static constexpr std::string_view typeOther = "other";
        static constexpr std::string_view typeGlobal = "global";

        static constexpr std::string_view topic = "topic";
        //static constexpr std::string_view logicalArg = "condition";

        static constexpr std::string_view exprVector = "action.assign";
        static constexpr std::string_view functioncalls_global = "action.functioncall.global";
        static constexpr std::string_view functioncalls_self = "action.functioncall.self";
        static constexpr std::string_view functioncalls_other = "action.functioncall.other";
    };

    // Keys within any Ruleset json object
    // No scope! They are at the root of any ruleset object within ruleset.broadcast[i]
    struct Ruleset {
        // TODO: Use these ones later on:
        //       Make sure to refactor any usage in json files
        static constexpr auto topic = Data::ScopedKey("topic");
        static constexpr auto condition = Data::ScopedKey("condition");
        // If both are met, do:
        static constexpr auto assignments = Data::ScopedKey("action.assign");
        static constexpr auto parseOnGlobal = Data::ScopedKey("action.functioncall.global");
        static constexpr auto parseOnSelf   = Data::ScopedKey("action.functioncall.self");
        static constexpr auto parseOnOther  = Data::ScopedKey("action.functioncall.other");
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
