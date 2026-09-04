//------------------------------------------
// Includes

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Graphics/DearImGui/Core.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Core {

bool checkImguiInitialized() {
    return ImGui::GetCurrentContext() != nullptr;
}

bool checkImguiReadyForRendering() {
    return checkImguiInitialized() && ImGui::GetFrameCount() > 0;
}

} // namespace Nebulite::Graphics::DearImGui::Core
