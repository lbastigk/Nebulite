/**
 * @file Drawcall.hpp
 * @brief This file contains the declaration of the Nebulite::Graphics::Drawcall base class.
 * @todo The big question currently is how to best implement the structure into a RenderObject.
 *       RenderObjects should be able to hold multiple Drawcalls, with a hashmap where the hashmap key is also the key in the JsonScope.
 *       This way, we can easily access and modify specific drawcalls within a RenderObject.
 *       E.g.: on-drawcall parse <args>
 *       But should each drawcall hold its Texture?
 *       It makes sense in a way, as any drawcall needs its own texture data.
 *       This way, a drawcall would seemingly be just a wrapper around a texture to better integrate with the rendering pipeline.
 *       It may be better to integrate this into each Texture, and have a Texture::draw() method instead.
 *       For now we implement a rudimentary Drawcall class to see how it works out.
 */

#ifndef NEBULITE_GRAPHICS_DRAWCALL_HPP
#define NEBULITE_GRAPHICS_DRAWCALL_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Core/Texture.hpp"
#include "Core/JsonScope.hpp"
#include "Utility/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Graphics {
/**
 * @class Nebulite::Graphics::Drawcall
 * @brief Base class for all draw calls in the rendering pipeline.
 */
class Drawcall {
public:
    // Any Drawcall is based on a scopes data
    explicit Drawcall(Core::JsonScope& workspace) : drawcallScope(workspace), texture(workspace) {
        updateDrawcallData();
        refs.initialize(workspace);
    }

    ~Drawcall() = default;

    void draw(float const& offsetX, float const& offsetY) const ;

    void update() { // TODO: Call in RenderObject update
        updaterRoutine.update();
    }

    // Parse a string onto the texture
    Constants::Error parseStr(std::string const& str);

protected:
    struct Key {
        static auto constexpr type = Data::ScopedKeyView("drawType");

        struct Rect {
            static auto constexpr srcX = Data::ScopedKeyView("rect.src.x");
            static auto constexpr srcY = Data::ScopedKeyView("rect.src.y");
            static auto constexpr srcW = Data::ScopedKeyView("rect.src.w");
            static auto constexpr srcH = Data::ScopedKeyView("rect.src.h");

            static auto constexpr dstX = Data::ScopedKeyView("rect.dst.x");
            static auto constexpr dstY = Data::ScopedKeyView("rect.dst.y");
            static auto constexpr dstW = Data::ScopedKeyView("rect.dst.w");
            static auto constexpr dstH = Data::ScopedKeyView("rect.dst.h");
        };



        // TODO: Use these in the Drawcall implementations, then remove them from Constants::KeyNames

        struct Sprite {
            static auto constexpr pixelSizeX = Data::ScopedKeyView("sprite.sizeX");
            static auto constexpr pixelSizeY = Data::ScopedKeyView("sprite.sizeY");
            static auto constexpr imageLocation = Data::ScopedKeyView("sprite.link");
            static auto constexpr isSpritesheet = Data::ScopedKeyView("sprite.spritesheet.isSpritesheet");
            static auto constexpr spritesheetSizeX = Data::ScopedKeyView("sprite.spritesheet.sizeX");
            static auto constexpr spritesheetSizeY = Data::ScopedKeyView("sprite.spritesheet.sizeY");
            static auto constexpr spritesheetOffsetX = Data::ScopedKeyView("sprite.spritesheet.offsetX");
            static auto constexpr spritesheetOffsetY = Data::ScopedKeyView("sprite.spritesheet.offsetY");

        };

        struct Text {
            static auto constexpr fontsize = Data::ScopedKeyView("text.fontSize");
            static auto constexpr str = Data::ScopedKeyView("text.str");
            static auto constexpr colorR = Data::ScopedKeyView("text.color.R");
            static auto constexpr colorG = Data::ScopedKeyView("text.color.G");
            static auto constexpr colorB = Data::ScopedKeyView("text.color.B");
            static auto constexpr colorA = Data::ScopedKeyView("text.color.A");
            static auto constexpr dx = Data::ScopedKeyView("text.dx");
            static auto constexpr dy = Data::ScopedKeyView("text.dy");
        };
    };

    struct Refs {
        // TODO: Add frequently used references here for faster access
        double* rectSrcX = nullptr;
        double* rectSrcY = nullptr;
        double* rectSrcW = nullptr;
        double* rectSrcH = nullptr;

        double* rectDstX = nullptr;
        double* rectDstY = nullptr;
        double* rectDstW = nullptr;
        double* rectDstH = nullptr;

        double* textDx = nullptr;
        double* textDy = nullptr;
        double* textColorR = nullptr;
        double* textColorG = nullptr;
        double* textColorB = nullptr;
        double* textColorA = nullptr;
        double* textFontsize = nullptr;

        void initialize(Core::JsonScope const& scope);
    } refs;

    enum Type {
        SPRITE,
        TEXT
    }type;

    Core::JsonScope& drawcallScope;
    Core::Texture texture; // Holds the data for the texture to draw
    SDL_Rect destRect{}; // Destination rectangle for drawing

    void updateDrawcallData();

    // Allows periodic updating of drawcall data to reflect current state
    Utility::TimedRoutine updaterRoutine{
        [this] {
            updateDrawcallData();
        },
        1000
    }; // Routine to update the drawcall data periodically

    //------------------------------------------
    // Specific initializers

    void initializeSprite();

    void initializeText();
};

// Idea for how the JsonScope may look like:
struct ExampleDrawCallData {
    std::string type; // e.g., "sprite", "text"
    // -> if we use a number, we could switch-case on it for faster processing
    // but that makes abstraction harder, so we may wish to keep it as a string for now

    // Specific to sprite
    std::string link;
    SDL_Rect srcRect;

    // Specific to text
    std::string text;
    int fontSize;
    SDL_Color color;
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_DRAWCALL_HPP
