#ifndef NEBULITE_GRAPHICS_DEARIMGUI_COLOR_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_COLOR_HPP

//------------------------------------------
// Includes

// External
#include <cstddef>

// External
#include <imgui.h>

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Color {
/**
 * @brief Splits the colour wheel into n segments and returns the i-th colour.
 * @param i The index of the colour to return
 * @param n The total number of colours to generate
 * @return The i-th colour in the colour wheel, as an ImVec4
 */
ImVec4 splitColorWheel(std::size_t i, std::size_t n);
} // namespace Nebulite::Graphics::DearImGui::Color
#endif // NEBULITE_GRAPHICS_DEARIMGUI_COLOR_HPP
