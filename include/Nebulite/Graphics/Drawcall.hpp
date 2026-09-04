#ifndef NEBULITE_GRAPHICS_DRAWCALL_HPP
#define NEBULITE_GRAPHICS_DRAWCALL_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <memory>
#include <string_view>

// External
#include <SDL3/SDL_rect.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/Texture.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Graphics/DrawType/DrawType.hpp"
#include "Nebulite/Graphics/DrawcallRefs.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

namespace Nebulite::Utility::Io {
class Capture;
} // namespace Nebulite::Utility::Io

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
 *          While the Texture class is able to parse commands that modify textures directly,
 *          drawcalls are responsible for the first initialization and presentation of textures:
 *          Drawcall -> create Texture and reinit on changes
 *          Texture -> provide Texture memory modifications and memory management
 */
class Drawcall {
public:
    // Any Drawcall is based on a scopes data
    explicit Drawcall(Data::JsonScope& workspace, Utility::Io::Capture& parentCapture);

    ~Drawcall() = default;

    Drawcall(Drawcall const&) = delete;
    Drawcall& operator=(Drawcall const&) = delete;
    Drawcall(Drawcall&&) = delete;
    Drawcall& operator=(Drawcall&&) = delete;

    void draw(Core::Renderer& nebuliteRenderer, float offsetX, float offsetY);

    void update();

    // Parse a string onto the texture
    [[nodiscard]] Constants::Event parseStr(std::string_view str, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const ;

    class ApplyDefault {
    public:
        /**
         * @brief Sets a default drawcall configuration for a sprite.
         * @param scope The JSON scope to set defaults in.
         */
        static void sprite(Data::JsonScope& scope);

        /**
         * @brief Sets a default drawcall configuration for text.
         * @param scope The JSON scope to set defaults in.
         */
        static void text(Data::JsonScope& scope);
    };
private:
    Core::Texture texture; // Texture domain
    Data::JsonScope& drawcallScope; // The owned scope of the drawcall, holding relevant data
    std::unique_ptr<DrawType::DrawType> toDraw; // Configuration helper of the type to draw
    Utility::Coordination::TimedRoutine updaterRoutine; // Allows periodic updating of drawcall data to reflect current state

    DrawcallRefs refs;

    SDL_FPoint rotationCenter{.x=0.0f, .y=0.0f};

    bool reInitializeRequested = false;

    // TODO: requestOverride, allowing us to redraw the texture every frame if needed

    enum class Type : std::uint8_t {
        sprite,
        text,
        circle,
        polygon,
        // More ideas:
        // - tiledSprite (set fixed size of each tile, or a min/max size, and tile the texture accordingly) Helpful for GUI elements
        //   > dstRect determines the overall area to fill
        //   > srcRect determines what part of the texture to tile
        //   > scalar (how much to scale each tile by)
        //   > tileType (what to do with partial tiles: cut-off, align-nearest-down, align-nearest-up, align-nearest)
        //     > cutoff: simply cut off any partial tiles at the edges
        //     > align-nearest: every tile is scaled so that only full tiles fit in the dstRect
        //       > e.g. if tile is 8x8 and dstRect is 20x20:
        //         scale tiles to 6.66x6.66 so that 3x3 tiles fit perfectly
        //         scale tiles to 10x10 so that 2x2 tiles fit perfectly
        //   > perhaps we should determine if we create a new texture for this or render directly from the original texture?
    } type = Type::sprite; // Default to sprite

    //------------------------------------------
    // Updater

    static auto constexpr updateDrawcallDataIntervalMs = 1000u; // Update every second
    static auto constexpr updateDrawcallDataIntervalJitterMs = static_cast<std::uint64_t>(0.2*updateDrawcallDataIntervalMs); // Add some jitter to avoid sync with other routines

    void updateDrawcallData();

    //------------------------------------------
    // Rendering

    void renderTexture(Core::Renderer const& nebuliteRenderer, float dX, float dY);
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_DRAWCALL_HPP
