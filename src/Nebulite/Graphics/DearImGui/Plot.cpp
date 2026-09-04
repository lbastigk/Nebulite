//------------------------------------------
// Includes

// External
#include <array>
#include <cstddef>
#include <cstdint> // NOLINT
#include <cstring> // NOLINT
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

// External
#include <imgui.h>
#include <implot.h>

// Nebulite
#include "Nebulite/Graphics/DearImGui/DomainViewer.hpp"
#include "Nebulite/Graphics/DearImGui/Plot.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Utility/Promise.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Plot {

Labels::Labels(std::uint16_t const id) {
    nameX = "##PlotX_" + std::to_string(id);
    nameY = "##PlotY_" + std::to_string(id);
    nameColour = "##PlotColor_" + std::to_string(id);
    remove = "Remove##PlotRemove_" + std::to_string(id);
}

PlotData::PlotData(std::uint16_t const id, ImVec4 const& plotColour)
    : expressionX{"$({global:time.t})"}
    , expressionY{"$(sin({global:time.t}))"}
    , colour{plotColour}
    , labels{id}{
    std::strncpy(bufX.data(), expressionX.getFullExpression().c_str(), bufX.size() - 1);
    std::strncpy(bufY.data(), expressionY.getFullExpression().c_str(), bufY.size() - 1);

    // Ensure null-termination
    bufX.back() = '\0';
    bufY.back() = '\0';
}

Plots::Plots()
    : expressionXMin("$({global:time.t} - 50)"), expressionXMax("$({global:time.t})")
    , expressionYMin("$(-2)"), expressionYMax("$(2)") {
    std::strncpy(bufXMin.data(), expressionXMin.getFullExpression().c_str(), 256);
    std::strncpy(bufXMax.data(), expressionXMax.getFullExpression().c_str(), 256);
    std::strncpy(bufYMin.data(), expressionYMin.getFullExpression().c_str(), 256);
    std::strncpy(bufYMax.data(), expressionYMax.getFullExpression().c_str(), 256);

    // Ensure null-termination
    bufXMin.back() = '\0';
    bufXMax.back() = '\0';
    bufYMin.back() = '\0';
    bufYMax.back() = '\0';
}

bool Plots::validLimits() const {
    return expressionXMin.isReturnableAsDouble()
        && expressionXMax.isReturnableAsDouble()
        && expressionYMin.isReturnableAsDouble()
        && expressionYMax.isReturnableAsDouble();
}

void Plots::update(Interaction::ContextScope const& ctxScope) {
    if (lastUpdated + updateInterval < ImGui::GetTime()) {
        lastUpdated = ImGui::GetTime();
        for (auto& plot : plots) {
            if (plot.expressionX.isReturnableAsDouble() && plot.expressionY.isReturnableAsDouble()) {
                auto constexpr promise = Nebulite::Utility::Promise<
                    Utility::PromiseType::FunctionVerified,
                    &Interaction::Logic::Expression::isReturnableAsDouble
                >{};
                double const x = plot.expressionX.evalAsDouble(ctxScope, promise);
                double const y = plot.expressionY.evalAsDouble(ctxScope, promise);
                plot.points.emplace_back(x, y);
                while (plot.points.size() > static_cast<size_t>(pointsToKeep)) {
                    plot.points.pop_front();
                }
            }
        }
    }
}

void Plots::plotList(std::list<std::pair<double, double>> const& points, ImVec4 const& colour){
    ImPlotSpec spec;
    spec.LineColor = colour;

    struct Getter {
        std::list<std::pair<double, double>>::const_iterator it;

        ImPlotPoint operator()(int const /*idx*/) {
            auto const& [x, y] = *it++;
            return ImPlotPoint{x, y};
        }
    };

    Getter getter{points.begin()};

    ImPlot::PlotLineG(
        "##Line",
        [](int const idx, void* data) -> ImPlotPoint {
            auto& get = *static_cast<Getter*>(data);
            return get(idx);
        },
        &getter,
        static_cast<int>(points.size()),
        spec
    );
}

void Plots::plot(Interaction::ContextScope const& ctxScope) {
    ImGui::Separator();
    if (ImPlot::BeginPlot("Plot", ImVec2(-1,0), ImPlotFlags_NoLegend)) {
        if (validLimits()) {
            auto constexpr promise = Nebulite::Utility::Promise<
                Utility::PromiseType::FunctionVerified,
                &Interaction::Logic::Expression::isReturnableAsDouble
            >{};
            double const xMin = expressionXMin.evalAsDouble(ctxScope, promise);
            double const xMax = expressionXMax.evalAsDouble(ctxScope, promise);
            double const yMin = expressionYMin.evalAsDouble(ctxScope, promise);
            double const yMax = expressionYMax.evalAsDouble(ctxScope, promise);
            ImPlot::SetupAxesLimits(xMin, xMax, yMin, yMax, ImPlotCond_Always);
        }

        for (auto& plot : plots) {
            plotList(plot.points, plot.colour);
        }
        ImPlot::EndPlot();
    }
}

void renderPlotData(int const columns, std::list<PlotData>& availablePlots) {
    for (auto [i, plot] : availablePlots | std::views::enumerate) {
        ImGui::Text("Plot %i:", static_cast<int>(i));
        if (ImGui::BeginTable("PlotPointsTable", columns+1, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders)) {
            // --- ROW 1 (X Data) ---
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(plot.colour));
            ImGui::Text("x");
            for (auto const x : plot.points | std::views::transform([](auto p) { return p.first; })) {
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", x);
            }

            // --- ROW 2 (Y Data) ---
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(plot.colour));
            ImGui::Text("y");
            for (auto const y : plot.points | std::views::transform([](auto p) { return p.second; })) {
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", y);
            }

            ImGui::EndTable();
        }
    }
}

} // namespace Nebulite::Graphics::DearImGui::Plot
