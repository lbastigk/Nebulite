#ifndef NEBULITE_GRAPHICS_DEARIMGUI_ALIGN_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_ALIGN_HPP

//------------------------------------------
// Includes

// Standard Library
#include <cstdint>

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Align {

enum class Alignment : std::uint8_t {
    none, // No automatic alignment, use exact position specified in windowPos
    top,
    bottom,
    left,
    right,
};

/**
 * @brief Imgui alignment helper, call before Imgui::Begin().
 * @param alignment The flags for the window alignment.
 */
void alignWindow(Alignment const& alignment);

/**
 * @brief Sets the cursor position to the right of the specified button.
 * @param buttonLabel The label of the button to align with.
 */
void setCursorPosXForRightAlignedButton(char const* buttonLabel);

} // namespace Nebulite::Graphics::DearImGui::Align
#endif // NEBULITE_GRAPHICS_DEARIMGUI_ALIGN_HPP
