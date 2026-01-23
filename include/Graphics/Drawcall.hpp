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

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Core/Texture.hpp"
#include "Core/JsonScope.hpp"

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
        if (auto const typeOnConstruction = drawcallScope.get<std::string>(Key::type, "sprite"); typeOnConstruction == "sprite") {
            type = SPRITE;
        }
        else if (typeOnConstruction == "text") {
            type = TEXT;
        }
    }

    ~Drawcall() = default;

    // Current idea: Depending on the type of drawcall, we have different draw implementations
    // Since there are different types of drawcalls, it might be better to have separate derived classes for each type
    // But it would be more complex, as any change in the workspace structure could potentially require the class to change.
    void draw();
protected:
    struct Key {
        static auto constexpr type = Data::ScopedKeyView::createUnscoped("type");
    };

    enum Type {
        SPRITE,
        TEXT
    }type;

    Core::JsonScope& drawcallScope;
    Core::Texture texture; // Holds the data for the texture to draw
    SDL_Rect destRect{}; // Destination rectangle for drawing
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
