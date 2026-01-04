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

#include "Core/JsonScope.hpp"

namespace Nebulite::Constants {

// TODO: Remove unused keys and refactor used ones
// TODO: move scope to private part of each struct when possible
struct KeyNames {
    struct Renderer {
        static auto constexpr scope = "renderer.";
        static auto constexpr dispResX = Data::ScopedKey::create<scope>("resolution.X");
        static auto constexpr dispResY = Data::ScopedKey::create<scope>("resolution.Y");
        static auto constexpr positionX = Data::ScopedKey::create<scope>("position.X");
        static auto constexpr positionY = Data::ScopedKey::create<scope>("position.Y");
    };

    struct GlobalSpace {
        static auto constexpr scope = "";

        // TODO: Unify with DomainModule::GlobalSpace::Time
        //       Currently duplicated keys here and in that module
        static auto constexpr time_t = Data::ScopedKey::create<scope>("time.t");
        static auto constexpr time_t_ms = Data::ScopedKey::create<scope>("time.t_ms");
        static auto constexpr time_dt = Data::ScopedKey::create<scope>("time.dt");
        static auto constexpr time_dt_ms = Data::ScopedKey::create<scope>("time.dt_ms");

        /**
         * @brief Keys for RNG (Random Number Generator) settings in GlobalSpace.
         * @todo Move to DomainModule::GlobalSpace::RNG, public so that other DomainModules can use it
         */
        struct RNG {
            static auto constexpr scope = "random.";
            static auto constexpr A = Data::ScopedKey::create<scope>("A");
            static auto constexpr B = Data::ScopedKey::create<scope>("B");
            static auto constexpr C = Data::ScopedKey::create<scope>("C");
            static auto constexpr D = Data::ScopedKey::create<scope>("D");
            static auto constexpr min = Data::ScopedKey::create<scope>("min");
            static auto constexpr max = Data::ScopedKey::create<scope>("max");
        };

        // Keys for Physics
        // TODO: Set all to specific values in GlobalSpace
        //       Perhaps even in a DomainModule for GlobalSpace,
        //       Then we can move these keys there
        struct Physics {
            static auto constexpr scope = "physics.";
            static auto constexpr G = Data::ScopedKey::create<scope>("G");
        };
    };

    struct RenderObject {
        // TODO: Use "" as scope, and modify keys to be relative to that scope
        static auto constexpr id = Data::ScopedKey("id");
        static auto constexpr positionX = Data::ScopedKey("posX");
        static auto constexpr positionY = Data::ScopedKey("posY");
        static auto constexpr layer = Data::ScopedKey("layer");

        // Keys for Ruleset invocations and subscriptions
        struct Ruleset {
            static auto constexpr scope = "ruleset.";
            static auto constexpr broadcast = Data::ScopedKey::create<scope>("broadcast");
            static auto constexpr listen = Data::ScopedKey::create<scope>("listen");
        };

        // TODO: Use "texture." as scope
        static auto constexpr pixelSizeX = Data::ScopedKey("sprite.sizeX");
        static auto constexpr pixelSizeY = Data::ScopedKey("sprite.sizeY");
        static auto constexpr imageLocation = Data::ScopedKey("sprite.link");
        static auto constexpr isSpritesheet = Data::ScopedKey("sprite.spritesheet.isSpritesheet");
        static auto constexpr spritesheetSizeX = Data::ScopedKey("sprite.spritesheet.sizeX");
        static auto constexpr spritesheetSizeY = Data::ScopedKey("sprite.spritesheet.sizeY");
        static auto constexpr spritesheetOffsetX = Data::ScopedKey("sprite.spritesheet.offsetX");
        static auto constexpr spritesheetOffsetY = Data::ScopedKey("sprite.spritesheet.offsetY");
        static auto constexpr textFontsize = Data::ScopedKey("text.fontSize");
        static auto constexpr textStr = Data::ScopedKey("text.str");
        static auto constexpr textColorR = Data::ScopedKey("text.color.R");
        static auto constexpr textColorG = Data::ScopedKey("text.color.G");
        static auto constexpr textColorB = Data::ScopedKey("text.color.B");
        static auto constexpr textColorA = Data::ScopedKey("text.color.A");
        static auto constexpr textDx = Data::ScopedKey("text.dx");
        static auto constexpr textDy = Data::ScopedKey("text.dy");
    };

    // Keys within any Ruleset JSON object
    // No scope! They are at the root of any ruleset object within ruleset.broadcast[i]
    struct Ruleset {
        // TODO: Use these ones later on:
        //       Make sure to refactor any usage in json files
        static auto constexpr topic = Data::ScopedKey("topic");
        static auto constexpr condition = Data::ScopedKey("condition");
        // If both are met, do:
        static auto constexpr assignments = Data::ScopedKey("action.assign");
        static auto constexpr parseOnGlobal = Data::ScopedKey("action.functioncall.global");
        static auto constexpr parseOnSelf   = Data::ScopedKey("action.functioncall.self");
        static auto constexpr parseOnOther  = Data::ScopedKey("action.functioncall.other");
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
