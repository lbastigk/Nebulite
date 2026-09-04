//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Graphics/DrawType/DrawType.hpp"
#include "Nebulite/Math/Vec2.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DrawType {

void DrawType::drawToTexture(Core::Renderer& /*renderer*/, Core::Texture& /*texture*/, Data::JsonScope& /*scope*/, DrawcallRefs& /*refs*/) {}

bool DrawType::diff(Data::JsonScope& /*scope*/, DrawcallRefs& /*refs*/) {
    return false;
}

Math::Vec2<float> DrawType::getRenderOffset(DrawcallRefs& /*refs*/) {
    return Math::Vec2<float>{
        .x=0.0f,
        .y=0.0f,
    };
}

} // namespace Nebulite::Graphics::DrawType
