#ifndef NEBULITE_GRAPHICS_DRAWTYPE_DRAWTYPE_HPP
#define NEBULITE_GRAPHICS_DRAWTYPE_DRAWTYPE_HPP

//------------------------------------------
// Includes

// Standard library
#include <utility>

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

    DrawType& operator=(const DrawType&) = default;
    DrawType& operator=(DrawType&&) = default;
    DrawType(const DrawType&) = default;
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

    virtual std::pair<float,float> getRenderOffset(DrawcallRefs& refs);
};
} // namespace Nebulite::Graphics::DrawType
#endif // NEBULITE_GRAPHICS_DRAWTYPE_DRAWTYPE_HPP
