//------------------------------------------
// Includes

// Standard Library
#include <utility>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Graphics/DearImGui/Align.hpp"

namespace Nebulite::Graphics::DearImGui::Align {

void setCursorPosXForRightAlignedButton(char const* buttonLabel) {
    float const buttonWidth = ImGui::CalcTextSize(buttonLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float const scrollbarWidth = ImGui::GetScrollMaxY() > 0.0f ? ImGui::GetStyle().ScrollbarSize : 0.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - buttonWidth - scrollbarWidth);
}

void alignWindow(Alignment const& alignment) {
    ImGuiViewport const* const vp = ImGui::GetMainViewport();

    ImVec2 const vpPos  = vp->WorkPos;
    ImVec2 const vpSize = vp->WorkSize;

    auto const topPos    = ImVec2(vpPos.x, vpPos.y);
    auto const bottomPos = ImVec2(vpPos.x, vpPos.y + vpSize.y);
    auto const leftPos   = ImVec2(vpPos.x, vpPos.y);
    auto const rightPos  = ImVec2(vpPos.x + vpSize.x, vpPos.y);

    switch (alignment) {
    case Alignment::top:
        ImGui::SetNextWindowPos(topPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x, vpSize.y * 0.5f),
            ImGuiCond_Always
        );
        break;
    case Alignment::bottom:
        ImGui::SetNextWindowPos(bottomPos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x, vpSize.y * 0.5f),
            ImGuiCond_Always
        );
        break;
    case Alignment::left:
        ImGui::SetNextWindowPos(leftPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x * 0.5f, vpSize.y),
            ImGuiCond_Always
        );
        break;
    case Alignment::right:
        ImGui::SetNextWindowPos(rightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x * 0.5f, vpSize.y),
            ImGuiCond_Always
        );
        break;
    case Alignment::none:
        break;
    default:
        std::unreachable();
    }
}

} // namespace Nebulite::Graphics::DearImGui::Align

