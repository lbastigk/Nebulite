#ifndef GRAPHICS_DRAWCALL_HPP
#define GRAPHICS_DRAWCALL_HPP

//------------------------------------------
// Includes

// Standard library
#include <SDL3/SDL_pixels.h>
#include <cstddef> // NOLINT
#include <cstdint> // NOLINT
#include <string>
#include <string_view>

// External
#include <SDL3/SDL_rect.h>
#include <SDL3_ttf/SDL_ttf.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/Texture.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/Capture.hpp"

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
    explicit Drawcall(Data::JsonScope& workspace, Utility::IO::Capture& parentCapture);

    ~Drawcall() = default;

    Drawcall(Drawcall const&) = delete;
    Drawcall& operator=(Drawcall const&) = delete;
    Drawcall(Drawcall&&) = delete;
    Drawcall& operator=(Drawcall&&) = delete;

    void draw(Core::Renderer const& nebuliteRenderer, float const& offsetX, float const& offsetY);

    void update();

    // Parse a string onto the texture
    [[nodiscard]] Constants::Event parseStr(std::string_view str, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const ;

    class ApplyDefault {
    public:
        /**
         * @brief Sets a default drawcall configuration for a sprite.
         * @param scope The JSON scope to set defaults in.
         */
        static void Sprite(Data::JsonScope& scope);

        /**
         * @brief Sets a default drawcall configuration for text.
         * @param scope The JSON scope to set defaults in.
         */
        static void Text(Data::JsonScope& scope);
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

        struct PolygonSpecific {
            static auto constexpr points = Data::ScopedKeyView("textureData.points"); // Array of point objects {x, y}
            static auto constexpr filled = Data::ScopedKeyView("textureData.filled"); // bool
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

        double* rotationDegrees = nullptr;
        double* rotationCenterX = nullptr;
        double* rotationCenterY = nullptr;

        double* colorR = nullptr;
        double* colorG = nullptr;
        double* colorB = nullptr;
        double* colorA = nullptr;

        double* textFontsize = nullptr;

        double* circleRadius = nullptr;

        double* polygonFilled = nullptr;

        void initialize(Data::JsonScope const& scope);
    } refs;

    // Important state for diff-logic
    // Perhaps changing to std::variant is better, since only one type is required
    struct State {
        struct Sprite {
            std::string link;
        } sprite;

        struct Text {
            std::string text;
            SDL_Color textColor{.r=0,.g=0,.b=0,.a=0};
        } text;

        struct Circle {
            int radius;
            SDL_Color circleColor;
        } circle;

        struct Polygon {
            std::size_t pointCount;
            SDL_Color polyColor{.r=0,.g=0,.b=0,.a=0};
        } polygon;
    } state;

    bool reInitializeRequested = false;

    SDL_FPoint rotationCenter{.x=0.0f, .y=0.0f};

    // TODO: requestOverride, allowing us to redraw the texture every frame if needed

    enum class Type : std::uint8_t {
        SPRITE,
        TEXT,
        CIRCLE,
        POLYGON
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
    }type = Type::SPRITE; // Default to sprite

    Data::JsonScope& drawcallScope;
    Core::Texture texture; // Holds the data for the texture to draw

    //------------------------------------------
    // Updater

    static auto constexpr updateDrawcallDataIntervalMs = 1000u; // Update every second
    static auto constexpr updateDrawcallDataIntervalJitterMs = static_cast<std::uint64_t>(0.2*updateDrawcallDataIntervalMs); // Add some jitter to avoid sync with other routines

    void updateDrawcallData();

    // Allows periodic updating of drawcall data to reflect current state
    Utility::Coordination::TimedRoutine updaterRoutine;

    //------------------------------------------
    // Rendering

    void renderTexture(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY);

    void renderText(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY);

    void renderSprite(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY);

    void renderCircle(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY);

    void renderPolygon(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY);

    //------------------------------------------
    // Specific initializers for each type

    /**
     * @brief Initializes the drawcall as a sprite.
     * @note Only called during the draw call, otherwise the thread safety would be compromised.
     */
    void initializeSprite();

    /**
     * @brief Sets standard text rect parameters in the drawcall scope if they are missing.
     * @param w The width of the texture
     * @param h The height of the texture
     * @param font The font used
     */
    void setStandardTextRectsIfMissing(float w, float h, TTF_Font* font) const ;

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

    /**
     * @brief Initializes the drawcall as a polygon.
     * @note Only called during the draw call, otherwise the thread safety would be compromised.
     */
    void initializePolygon();

    //------------------------------------------
    // Diff noticers for reinitialization

    bool diffSprite() const ;

    bool diffText() const ;

    bool diffCircle() const ;

    bool diffPolygon() const ;
};

} // namespace Nebulite::Graphics
#endif // GRAPHICS_DRAWCALL_HPP
