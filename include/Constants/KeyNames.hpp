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

#include "Data/Document/ScopedKey.hpp"

namespace Nebulite::DomainModule {
class Initializer; // Forward declaration of Initializer for friend declaration in DECLARE_SCOPE
} // namespace Nebulite::DomainModule

namespace Nebulite::Constants {

// Macro to help declare a private scope member. Must be the full scope!
#define DECLARE_SCOPE(scopeStr) static auto constexpr scope = scopeStr;

// Macro to help create a scoped key with the previously declared scope
#define MAKE_SCOPED(keyStr) ( (void)scope, Data::ScopedKeyView::create<scope>(keyStr) )

// TODO: Remove unused keys and refactor used ones
// TODO: move scope to private part of each struct when possible
struct KeyNames {
    struct Renderer {
        DECLARE_SCOPE("renderer.")
        static auto constexpr dispResX = MAKE_SCOPED("resolution.X");
        static auto constexpr dispResY = MAKE_SCOPED("resolution.Y");
        static auto constexpr dispResXLogical = MAKE_SCOPED("resolution.logical.X");
        static auto constexpr dispResYLogical = MAKE_SCOPED("resolution.logical.Y");
        static auto constexpr positionX = MAKE_SCOPED("position.X");
        static auto constexpr positionY = MAKE_SCOPED("position.Y");
        static auto constexpr windowScale = MAKE_SCOPED("resolution.scalar");
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
            static auto constexpr list = MAKE_SCOPED("list");
            static auto constexpr listen = MAKE_SCOPED("listen");
        };

        static auto constexpr draw = Data::ScopedKeyView("draw");

        static auto constexpr sizeX = Data::ScopedKeyView("size.x");
        static auto constexpr sizeY = Data::ScopedKeyView("size.y");
        static auto constexpr sizeR = Data::ScopedKeyView("size.r"); // TODO: Make sure any collision modules are able to use (X,Y) and radius
    };

    // Keys within any Ruleset JSON object
    // No scope! They are at the root of any ruleset object within ruleset.broadcast[i]
    struct Ruleset {
        // TODO: Use these ones later on:
        //       Make sure to refactor any usage in json files
        static auto constexpr topic = Data::ScopedKeyView("topic");
        static auto constexpr condition = Data::ScopedKeyView("condition");
        // If both are met, do:
        static auto constexpr assignments = Data::ScopedKeyView("action.assign");
        static auto constexpr parseOnGlobal = Data::ScopedKeyView("action.functioncall.global");
        static auto constexpr parseOnSelf   = Data::ScopedKeyView("action.functioncall.self");
        static auto constexpr parseOnOther  = Data::ScopedKeyView("action.functioncall.other");
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
