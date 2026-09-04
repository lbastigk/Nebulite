#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Data/Document/ScopePattern.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"

//------------------------------------------
namespace Nebulite::Constants {
/**
 * @struct KeyNames
 * @brief Centralized struct for constant key names used throughout the Nebulite framework.
 * @details While for DomainModules we can write scoped keys directly within their classes,
 *          this is not feasible for Domain-Related keys due to their hierarchical nature.
 *          Specifically, DomainModules are never dependent on other DomainModules besides their key names.
 *          But Domains can own other Domains, requiring knowledge of their key names.
 *          Therefore, we centralize these key names here to avoid circular dependencies
 *          and to ensure consistency across the framework.
 */
struct KeyNames {
    /**
     * @brief Basic keys related to any domain.
     * @details No scope! As Domains may be inside other Domains, the scope is arbitrary.
     */
    struct Domain : Data::KeyGroup<Data::ScopePattern::noScope> {
        //static auto constexpr id = makeScoped("id");
    };

    /**
     * @struct Drawcall
     * @brief Holds the keys used in the drawcall JsonScope.
     * @details All keys are unscoped, as they are relative to the drawcall's own scope.
     *          Since the drawcall's scope isn't fixed within the RenderObject's document,
     *          we cannot use fully scoped keys here.
     *          Use these keys with caution, ensuring the scope you use them with is indeed
     *          the drawcall's / texture's scope!
     */
    struct Drawcall {
        static auto constexpr type = Data::ScopedKeyView("drawType"); // "sprite", "text", "geometry", etc.
        static auto constexpr textureData = Data::ScopedKeyView("textureData"); // Texture data object
        static auto constexpr rotationDegrees = Data::ScopedKeyView("textureData.rotation.angle"); // Rotation in degrees
        static auto constexpr rotationCenterX = Data::ScopedKeyView("textureData.rotation.center.x"); // Rotation center X
        static auto constexpr rotationCenterY = Data::ScopedKeyView("textureData.rotation.center.y"); // Rotation center Y

        struct Rect {
            static auto constexpr src = Data::ScopedKeyView("rect.src");
            static auto constexpr srcX = Data::ScopedKeyView("rect.src.x");
            static auto constexpr srcY = Data::ScopedKeyView("rect.src.y");
            static auto constexpr srcW = Data::ScopedKeyView("rect.src.w");
            static auto constexpr srcH = Data::ScopedKeyView("rect.src.h");

            static auto constexpr dst = Data::ScopedKeyView("rect.dst");
            static auto constexpr dstX = Data::ScopedKeyView("rect.dst.x");
            static auto constexpr dstY = Data::ScopedKeyView("rect.dst.y");
            static auto constexpr dstW = Data::ScopedKeyView("rect.dst.w");
            static auto constexpr dstH = Data::ScopedKeyView("rect.dst.h");
        };

        struct Color {
            static auto constexpr r = Data::ScopedKeyView("textureData.color.r");
            static auto constexpr g = Data::ScopedKeyView("textureData.color.g");
            static auto constexpr b = Data::ScopedKeyView("textureData.color.b");
            static auto constexpr a = Data::ScopedKeyView("textureData.color.a");
        };
    };

    /**
     * @struct Renderer
     * @brief Keys related to the Renderer domain
     * @details The scope is set to "renderer.", meaning the entire renderer lives inside this scope of the GlobalSpace.
     */
    struct Renderer : Data::KeyGroup<"renderer."> {
        static auto constexpr dispResXWindow = makeScoped("resolution.window.X");
        static auto constexpr dispResYWindow = makeScoped("resolution.window.Y");
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
    };

    /**
     * @brief Keys related to Rulesets, which define the behavior of RenderObjects and other entities.
     * @details No scope, as they are extracted from larger JSON objects and have arbitrary scopes depending on where they are defined.
     *          E.g. `ruleset.list[0]` for the first one, `ruleset.list[1]` for the second one, etc.
     */
    struct Ruleset : Data::KeyGroup<Data::ScopePattern::noScope> {
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
