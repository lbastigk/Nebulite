#ifndef NEBULITE_GRAPHICS_SDL_CANVAS_HPP
#define NEBULITE_GRAPHICS_SDL_CANVAS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>

// External
#include "SDL3/SDL_render.h"

//------------------------------------------
namespace Nebulite::Graphics::Sdl::Canvas {

/**
 * @brief Calls a given function f while the SDL render target is set to a given texture.
 * @details Restores the render target after the invocation.
 * @tparam F The Function type to call.
 * @param renderer The SDL renderer.
 * @param texture The texture to set as a render target.
 * @param f The function to call.
 */
template<typename F>
void drawOnTexture(SDL_Renderer* renderer, SDL_Texture* texture, F&& f) {
    assert(renderer != nullptr);
    assert(texture != nullptr);
    static_assert(std::is_invocable_r_v<void,F>, "F must be invocable with f()");

    auto* currentTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    std::invoke(std::forward<F>(f));
    SDL_SetRenderTarget(renderer, currentTarget);
}

} // namespace Nebulite::Graphics::Sdl::Canvas
#endif // NEBULITE_GRAPHICS_SDL_CANVAS_HPP
