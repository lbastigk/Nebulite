#ifndef NEBULITE_GRAPHICS_DRAWTYPE_DRAWTYPE_HPP
#define NEBULITE_GRAPHICS_DRAWTYPE_DRAWTYPE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Math/Vec2.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
class Texture;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Graphics {
struct DrawcallRefs;
} // namespace Nebulite::Graphics

//------------------------------------------
namespace Nebulite::Graphics::DrawType {
/**
 * @brief Defines a draw type object
 */
class DrawType {
public:
    DrawType() = default;
    virtual ~DrawType() = default;

    DrawType& operator=(DrawType const&) = default;
    DrawType& operator=(DrawType&&) = default;
    DrawType(DrawType const&) = default;
    DrawType(DrawType&&) = default;

    /**
     * @brief Draws the DrawType to a Texture
     * @param renderer The Renderer to use
     * @param texture The texture to draw on
     * @param scope The scope owning the data of the drawtype.
     * @param refs General references for any drawtype
     * @todo Add refs to DrawType in general, as well as JsonScope&
     */
    virtual void drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& refs);

    /**
     * @brief Signals that the linked values changed and the texture needs to be redrawn.
     * @return True if the texture needs to be redrawn with drawToTexture, false otherwise.
     */
    virtual bool diff(Data::JsonScope& scope, DrawcallRefs& refs);

    /**
     * @brief Returns the render offset to use for the specialized DrawType.
     * @param refs General references for any drawtype.
     * @return The offset.
     */
    virtual Math::Vec2<float> getRenderOffset(DrawcallRefs& refs);
};
} // namespace Nebulite::Graphics::DrawType
#endif // NEBULITE_GRAPHICS_DRAWTYPE_DRAWTYPE_HPP
