/**
 * @file SdlPrimitive.hpp
 * @brief This file contains SDL primitive drawing functions.
 */

#ifndef NEBULITE_GRAPHICS_SDL_PRIMITIVE_HPP
#define NEBULITE_GRAPHICS_SDL_PRIMITIVE_HPP

//------------------------------------------
// Includes

// Standard library
#include <vector>

// External
#include <SDL3/SDL.h>

//------------------------------------------

namespace Nebulite::Graphics {
class SdlPrimitive {
public:
    /**
     * @brief Draws a filled circle onto the given texture using the provided renderer.
     * @param renderer The SDL renderer to use for drawing.
     * @param texture The SDL texture to draw onto.
     * @param color The color of the circle.
     * @param radius The radius of the circle.
     */
    static void drawFilledCircle(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, int const& radius);

    /**
     * @brief Draws connected lines between the given points onto the specified texture using the provided renderer.
     * @param renderer The SDL renderer to use for drawing.
     * @param texture The SDL texture to draw onto.
     * @param color The color of the lines.
     * @param points A vector of SDL_FPoint representing the points to connect with lines.
     */
    static void drawLines(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, std::vector<SDL_FPoint> const& points);

    /**
     * @brief Draws an empty polygon defined by the given points onto the specified texture using the provided renderer.
     * @param renderer The SDL renderer to use for drawing.
     * @param texture The SDL texture to draw onto.
     * @param color The color of the polygon outline.
     * @param points A vector of SDL_FPoint representing the vertices of the polygon.
     */
    static void drawEmptyPolygon(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, std::vector<SDL_FPoint> const& points);

    /**
     * @brief Draws a filled polygon defined by the given points onto the specified texture using the provided renderer.
     * @param renderer The SDL renderer to use for drawing.
     * @param texture The SDL texture to draw onto.
     * @param color The color of the filled polygon.
     * @param points A vector of SDL_FPoint representing the vertices of the polygon.
     */
    static void drawFilledPolygon(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Color const& color, std::vector<SDL_FPoint> const& points);
};
} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_SDL_PRIMITIVE_HPP
