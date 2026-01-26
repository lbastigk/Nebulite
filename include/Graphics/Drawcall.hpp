/**
 * @file Drawcall.hpp
 * @brief This file contains the declaration of the Nebulite::Graphics::Drawcall base class.
 */

#ifndef NEBULITE_GRAPHICS_DRAWCALL_HPP
#define NEBULITE_GRAPHICS_DRAWCALL_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Core/Texture.hpp"
#include "Core/JsonScope.hpp"
#include "Utility/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Graphics {
/**
 * @class Nebulite::Graphics::Drawcall
 * @brief Base class for all draw calls in the rendering pipeline.
 * @details Drawcalls hold their own configuration data in a JsonScope,
 *          that is shared with a texture domain that it owns.
 *          Each drawcall is able to forward arguments to the texture it owns,
 *          allowing for dynamic texture updates based on user-defined strings.
 *          Drawcalls properly manage texture metadata updates such as source/destination rectangles.
 * @todo Legacy drawcall system has been removed, but the data still exists inside all json documents.
 *       We need to merge that data into the new system for all existing tests to actually show something.
 */
class Drawcall {
public:
    // Any Drawcall is based on a scopes data
    explicit Drawcall(Core::JsonScope& workspace);

    ~Drawcall() = default;

    void draw(float const& offsetX, float const& offsetY);

    void update();

    // Parse a string onto the texture
    Constants::Error parseStr(std::string const& str);

    class ApplyDefault {
    public:
        /**
         * @brief Sets a default drawcall configuration for a sprite.
         * @param scope The JSON scope to set defaults in.
         */
        static void Sprite(Core::JsonScope& scope);

        /**
         * @brief Sets a default drawcall configuration for text.
         * @param scope The JSON scope to set defaults in.
         */
        static void Text(Core::JsonScope& scope);
    };

    /**
     * @struct Key
     * @brief Holds the keys used in the drawcall JsonScope.
     * @details All keys are unscoped, as they are relative to the drawcall's own scope.
     *          Since the drawcall's scope isn't fixed within the RenderObject's document,
     *          we cannot use fully scoped keys here.
     *          Use these keys with caution, ensuring the scope you use them with is indeed
     *          the drawcall's / texture's scope!
     */
    struct Key {
        static auto constexpr type = Data::ScopedKeyView("drawType"); // "sprite", "text", "geometry", etc.

        static auto constexpr textureData = Data::ScopedKeyView("textureData"); // Texture data object

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

        // TODO: Use these in the Drawcall implementations, then remove them from Constants::KeyNames

        struct SpriteSpecific {
            static auto constexpr imageLocation = Data::ScopedKeyView("textureData.link");
        };

        struct Color {
            static auto constexpr R = Data::ScopedKeyView("textureData.color.r");
            static auto constexpr G = Data::ScopedKeyView("textureData.color.g");
            static auto constexpr B = Data::ScopedKeyView("textureData.color.b");
            static auto constexpr A = Data::ScopedKeyView("textureData.color.a");
        };

        struct TextSpecific {
            static auto constexpr fontsize = Data::ScopedKeyView("textureData.fontSize");
            static auto constexpr str = Data::ScopedKeyView("textureData.str");

        };

        struct CircleSpecific {
            static auto constexpr radius = Data::ScopedKeyView("textureData.radius");
        };
    };

private:
    /**
     * @struct Refs
     * @brief Holds frequently used references for quick access.
     */
    struct Refs {
        double* rectSrcX = nullptr;
        double* rectSrcY = nullptr;
        double* rectSrcW = nullptr;
        double* rectSrcH = nullptr;

        double* rectDstX = nullptr;
        double* rectDstY = nullptr;
        double* rectDstW = nullptr;
        double* rectDstH = nullptr;

        double* colorR = nullptr;
        double* colorG = nullptr;
        double* colorB = nullptr;
        double* colorA = nullptr;

        double* textFontsize = nullptr;

        double* circleRadius = nullptr;

        void initialize(Core::JsonScope const& scope);
    } refs;

    bool reInitializeRequested = false;

    enum Type {
        SPRITE,
        TEXT,
        CIRCLE
        // More ideas:
        // - GEOMETRY
        // - tiledSprite (set fixed size of each tile, or a min/max size, and tile the texture accordingly) Helpful for GUI elements
    }type;

    Core::JsonScope& drawcallScope;
    Core::Texture texture; // Holds the data for the texture to draw

    //------------------------------------------
    // Updater

    static auto constexpr updateDrawcallDataIntervalMs = 1000u; // Update every second
    static auto constexpr updateDrawcallDataIntervalJitterMs = static_cast<uint64_t>(0.2*updateDrawcallDataIntervalMs); // Add some jitter to avoid sync with other routines

    void updateDrawcallData();

    // Allows periodic updating of drawcall data to reflect current state
    Utility::TimedRoutine updaterRoutine;

    //------------------------------------------
    // Specific initializers for each type

    /**
     * @brief Initializes the drawcall as a sprite.
     * @note Only called during the draw call, otherwise the thread safety would be compromised.
     */
    void initializeSprite();

    /**
     * @brief Initializes the drawcall as a text.
     * @note Only called during the draw call, otherwise the thread safety would be compromised.
     */
    void initializeText();

    /**
     * @brief Initializes the drawcall as a circle.
     * @note Only called during the draw call, otherwise the thread safety would be compromised.
     */
    void initializeCircle();
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_DRAWCALL_HPP
