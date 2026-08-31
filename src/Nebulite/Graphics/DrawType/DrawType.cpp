//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "Nebulite/Graphics/DrawType/DrawType.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DrawType {

void DrawType::drawToTexture(Core::Renderer& /*renderer*/, Core::Texture& /*texture*/, Data::JsonScope& /*scope*/, DrawcallRefs& /*refs*/) {}

bool DrawType::diff(Data::JsonScope& /*scope*/, DrawcallRefs& /*refs*/) {
    return false;
}

std::pair<float,float> DrawType::getRenderOffset(DrawcallRefs& /*refs*/) {
    return {0.0, 0.0};
}

} // namespace Nebulite::Graphics::DrawType
