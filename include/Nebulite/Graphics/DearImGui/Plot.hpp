#ifndef NEBULITE_GRAPHICS_DEARIMGUI_PLOT_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_PLOT_HPP

//------------------------------------------
// Includes

// External
#include <array>
#include <cfloat>
#include <cstddef>
#include <cstdint> // NOLINT
#include <cstring> // NOLINT
#include <list>
#include <string>
#include <utility>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Graphics/DearImGui/DomainViewer.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Plot {

struct Labels {
    std::string nameX;
    std::string nameY;
    std::string nameColour;
    std::string remove;

    explicit Labels(std::uint16_t id);
};

struct PlotData {
    std::array<char, 256> bufX{};
    std::array<char, 256> bufY{};
    Interaction::Logic::Expression expressionX;
    Interaction::Logic::Expression expressionY;
    std::list<std::pair<double, double>> points;
    ImVec4 colour;
    Labels labels;

    PlotData(std::uint16_t id, ImVec4 const& plotColour);
};

struct Plots {
    std::uint16_t idCounter = 0;
    std::list<PlotData> plots;

    int pointsToKeep = 200;

    double lastUpdated = 0; // seconds
    static double constexpr updateInterval = 0.01; // seconds
    static double constexpr removeInterval = 120; // seconds

    std::array<char, 256> bufXMin = {};
    std::array<char, 256> bufXMax = {};
    std::array<char, 256> bufYMin = {};
    std::array<char, 256> bufYMax = {};

    Interaction::Logic::Expression expressionXMin;
    Interaction::Logic::Expression expressionXMax;

    Interaction::Logic::Expression expressionYMin;
    Interaction::Logic::Expression expressionYMax;

    Plots();

    bool validLimits() const ;

    void update(Interaction::ContextScope const& ctxScope);

    static void plotList(std::list<std::pair<double, double>> const& points, ImVec4 const& colour);

    void plot(Interaction::ContextScope const& ctxScope);
};

void renderPlotData(int columns, std::list<PlotData>& availablePlots);

template<std::size_t N = 256>
bool expressionInput(std::array<char, N>& buf, Interaction::Logic::Expression& expr, char const* label) {
    static auto constexpr yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    static auto constexpr red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    bool const pushedColour = [&] {
        // Faulty expression
        if (!expr.isReturnableAsDouble() || expr.getFullExpression().size() > buf.size()) {
            ImGui::PushStyleColor(ImGuiCol_Text, red);
            return true;
        }
        // Unsaved expression
        if (buf.data() != expr.getFullExpression()) {
            ImGui::PushStyleColor(ImGuiCol_Text, yellow);
            return true;
        }
        return false;
    }();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText(label, buf.data(), buf.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string exprStr = buf.data();
        if (!exprStr.starts_with("$(")) {
            exprStr = "$(" + exprStr + ")";
            std::strncpy(buf.data(), exprStr.c_str(), buf.size() - 1);
            buf.back() = '\0'; // Ensure buf is null-terminated
        }
        expr = Interaction::Logic::Expression{exprStr};
        if (pushedColour) {
            ImGui::PopStyleColor();
        }
        return true;
    }
    if (pushedColour) {
        ImGui::PopStyleColor();
    }
    return false;
}

} // namespace Nebulite::Graphics::DearImGui::Plot
#endif // NEBULITE_GRAPHICS_DEARIMGUI_PLOT_HPP
