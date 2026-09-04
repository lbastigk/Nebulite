#ifndef NEBULITE_GRAPHICS_DRAWCALLREFS_HPP
#define NEBULITE_GRAPHICS_DRAWCALLREFS_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"

namespace Nebulite::Graphics {
/**
 * @struct DrawcallRefs
 * @brief Holds frequently used references for quick access.
 */
struct DrawcallRefs {
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

    void initialize(Data::JsonScope const& scope){
        // Source Rect
        rectSrcX = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::srcX);
        rectSrcY = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::srcY);
        rectSrcW = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::srcW);
        rectSrcH = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::srcH);

        // Destination Rect
        rectDstX = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::dstX);
        rectDstY = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::dstY);
        rectDstW = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::dstW);
        rectDstH = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Rect::dstH);

        // Rotation
        rotationDegrees = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::rotationDegrees);
        rotationCenterX = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::rotationCenterX);
        rotationCenterY = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::rotationCenterY);

        // Color
        colorR = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Color::r);
        colorG = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Color::g);
        colorB = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Color::b);
        colorA = scope.getStableDoublePointer(Constants::KeyNames::Drawcall::Color::a);
    }
};
} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_DRAWCALLREFS_HPP
