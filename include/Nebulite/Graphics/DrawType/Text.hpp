#ifndef NEBULITE_GRAPHICS_DRAWTYPE_TEXT_HPP
#define NEBULITE_GRAPHICS_DRAWTYPE_TEXT_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <utility>

// External
#include <SDL3/SDL_pixels.h>
#include <SDL3_ttf/SDL_ttf.h>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Graphics/DrawType/DrawType.hpp"

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
 * @brief Specialization for drawing Text
 */
class Text : public DrawType {
    std::string text;
    SDL_Color textColor{.r=0,.g=0,.b=0,.a=0};
    double* textFontsize = nullptr;
    
    void setStandardTextRectsIfMissing(Data::JsonScope& scope, float w, float h, TTF_Font* font) const ;
    
public:
    struct Key {
        static auto constexpr fontsize = Data::ScopedKeyView("textureData.fontSize");
        static auto constexpr str = Data::ScopedKeyView("textureData.str");
    };

    // TODO: Proper width wrapping based on fontsize and max width
    Text(Data::JsonScope& scope, DrawcallRefs& refs);

    void drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& refs) override ;

    bool diff(Data::JsonScope& scope, DrawcallRefs& refs) override ;

    std::pair<float,float> getRenderOffset(DrawcallRefs& /*refs*/) override ;
};
} // namespace Nebulite::Graphics::DrawType
#endif // NEBULITE_GRAPHICS_DRAWTYPE_TEXT_HPP
