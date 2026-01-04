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
 * @todo However, with enough forward declarations this might work and we can get rid of this file entirely.
 * @todo - Sort keys into their respective scopes and Domains
 *       - Refactor DomainModule keys into DomainModules instead of having them here
 *       - Turn all into Data::ScopedKey
 *       - Add scopes where applicable to improve safety
 */

#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

#include "Core/JsonScope.hpp"

namespace Nebulite::Constants {

// Macro to help declare a scope as private static constexpr member
#define DECLARE_SCOPE(scopeStr) private: static auto constexpr scope = scopeStr; public:

// Macro to help create a scoped key with the previously declared scope
#define MAKE_SCOPED(keyStr) ( (void)scope, Data::ScopedKey::create<scope>(keyStr) )

// TODO: Remove unused keys and refactor used ones
// TODO: move scope to private part of each struct when possible
struct KeyNames {
    struct Renderer {
        DECLARE_SCOPE("renderer.")
        static auto constexpr dispResX = MAKE_SCOPED("resolution.X");
        static auto constexpr dispResY = MAKE_SCOPED("resolution.Y");
        static auto constexpr positionX = MAKE_SCOPED("position.X");
        static auto constexpr positionY = MAKE_SCOPED("position.Y");
    };

    struct GlobalSpace {
        // No keys for now
    };

    struct RenderObject {
        DECLARE_SCOPE("")
        static auto constexpr id = MAKE_SCOPED("id");
        static auto constexpr positionX = MAKE_SCOPED("posX");
        static auto constexpr positionY = MAKE_SCOPED("posY");
        static auto constexpr layer = MAKE_SCOPED("layer");

        // Keys for Ruleset invocations and subscriptions
        struct Ruleset {
            DECLARE_SCOPE("ruleset.")
            static auto constexpr broadcast = MAKE_SCOPED("broadcast");
            static auto constexpr listen = MAKE_SCOPED("listen");
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
