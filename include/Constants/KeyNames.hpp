/**
 * @file KeyNames.hpp
 * @brief Defines constant key names used throughout the Nebulite framework.
 * @details While for DomainModules we can write scoped keys directly within their classes,
 *          this is not feasible for Domain-Related keys due to their hierarchical nature.
 *          Specifically, DomainModules are never dependent on other DomainModules besides their key names.
 *          But Domains can own other Domains, requiring knowledge of their key names.
 *          Therefore, we centralize these key names here to avoid circular dependencies
 *          and to ensure consistency across the framework.
 */

#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/KeyGroup.hpp"
#include "Data/Document/ScopedKey.hpp"

//------------------------------------------
namespace Nebulite::Constants {

/**
 * @struct KeyNames
 * @brief Centralized struct for constant key names used throughout the Nebulite framework.
 */
struct KeyNames {
    /**
     * @brief Basic keys related to any domain.
     * @details No scope! As Domains may be inside other Domains, the scope is arbitrary.
     */
    struct Domain : Data::KeyGroup<Data::ScopedKey::noScope> {
        //static auto constexpr id = makeScoped("id");
    };

    /**
     * @struct Renderer
     * @brief Keys related to the Renderer domain
     * @details The scope is set to "renderer.", meaning the entire renderer lives inside this scope of the GlobalSpace.
     */
    struct Renderer : Data::KeyGroup<"renderer."> {
        static auto constexpr dispResX = makeScoped("resolution.X");
        static auto constexpr dispResY = makeScoped("resolution.Y");
        static auto constexpr dispResXLogical = makeScoped("resolution.logical.X");
        static auto constexpr dispResYLogical = makeScoped("resolution.logical.Y");
        static auto constexpr positionX = makeScoped("position.X");
        static auto constexpr positionY = makeScoped("position.Y");
        static auto constexpr windowScale = makeScoped("resolution.scalar");
    };

    /**
     * @brief Keys in the global space.
     * @details The scope is the root scope.
     */
    struct GlobalSpace : Data::KeyGroup<""> {
        // No keys for now
    };

    /**
     * @brief Keys related to RenderObjects, which are the main entities in the Renderer domain.
     * @details The scope is the root scope, as RenderObjects own their own scope.
     */
    struct RenderObject : Data::KeyGroup<""> {
        static auto constexpr positionX = makeScoped("posX");
        static auto constexpr positionY = makeScoped("posY");
        static auto constexpr layer = makeScoped("layer");

        static auto constexpr draw = makeScoped("draw");
        static auto constexpr sizeX = makeScoped("size.x");
        static auto constexpr sizeY = makeScoped("size.y");
        static auto constexpr sizeR = makeScoped("size.r");

        // Keys for Ruleset invocations and subscriptions
        struct Ruleset : KeyGroup<"ruleset."> {
            static auto constexpr list = makeScoped("list");
            static auto constexpr listen = makeScoped("listen");
        };
    };

    /**
     * @brief Keys related to Rulesets, which define the behavior of RenderObjects and other entities.
     * @details No scope, as they are extracted from larger JSON objects and have arbitrary scopes depending on where they are defined.
     *          E.g. `ruleset.list[0]` for the first one, `ruleset.list[1]` for the second one, etc.
     */
    struct Ruleset : Data::KeyGroup<Data::ScopedKey::noScope> {
        static auto constexpr topic = makeScoped("topic");
        static auto constexpr condition = makeScoped("condition");
        static auto constexpr assignments = makeScoped("action.assign");
        static auto constexpr parseOnGlobal = makeScoped("action.functioncall.global");
        static auto constexpr parseOnSelf   = makeScoped("action.functioncall.self");
        static auto constexpr parseOnOther  = makeScoped("action.functioncall.other");
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
