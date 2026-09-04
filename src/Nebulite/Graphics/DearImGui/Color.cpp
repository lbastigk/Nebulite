//------------------------------------------
// Includes

// External
#include <cstddef>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Graphics/DearImGui/Color.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Color {

ImVec4 splitColorWheel(std::size_t const i, std::size_t const n) {
    float const hue = static_cast<float>(i % n) / static_cast<float>(n);

    ImVec4 color;
    ImGui::ColorConvertHSVtoRGB(
        hue,
        0.8f, // saturation
        0.9f, // value
        color.x,
        color.y,
        color.z
    );

    color.w = 1.0f;
    return color;
}

} // namespace Nebulite::Graphics::DearImGui::Color
