#ifndef NEBULITE_GRAPHICS_DEARIMGUI_CORE_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_CORE_HPP

//------------------------------------------
// Includes

// Standard Library
#include <array>
#include <cstddef>
#include <functional>
#include <utility>

// External
#include <imgui.h>

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Core {

/**
 * @brief Checks if imgui is initialized and ready for rendering.
 * @return true if imgui is initialized, false otherwise.
 */
bool checkImguiInitialized();

/**
 * @brief Checks if Imgui is ready to render a frame
 * @return true if imgui is initialized and a frame is started, false otherwise.
 */
bool checkImguiReadyForRendering();

/**
 * @brief Creates a child window in ImGui.
 * @param name The name of the child window.
 * @param f The function to execute within the child window.
 * @param size The size of the child window.
 * @param flags The flags for the child window.
 */
template<typename F>
void renderChild(char const* name, F&& f, ImVec2 const size = ImVec2(0, 0), ImGuiChildFlags const flags = ImGuiChildFlags_None) {
    if (ImGui::BeginChild(name, size, flags)) {
        std::invoke(std::forward<F>(f));
    }
    ImGui::EndChild();
}

/**
 * @brief Renders a table based on the provided data.
 * @tparam N The amount of rows.
 * @tparam F The content of the table.
 * @param label The label of the tabel.
 * @param c The header.
 * @param f The generator function for the entries.
 */
template<std::size_t N, typename F>
void renderTable(char const* label, std::array<char const*, N> c, F&& f) {
    if (ImGui::BeginTable(label, c.size(), ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders)) {
        ImGui::TableNextRow();
        for (auto const& desc : c) {
            ImGui::TableNextColumn();
            ImGui::Text("%s", desc);
        }
        std::invoke(std::forward<F>(f));
        ImGui::EndTable();
    }
}

} // namespace Nebulite::Graphics::DearImGui::Core
#endif // NEBULITE_GRAPHICS_DEARIMGUI_CORE_HPP
