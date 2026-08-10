//------------------------------------------
// Includes

// External
#include <algorithm>
#include <array>
#include <cfloat>
#include <cstddef>
#include <cstdint> // NOLINT
#include <cstring> // NOLINT
#include <functional>
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

// External
#include <imgui.h>
#include <implot.h>

// Nebulite
#include "Nebulite/Graphics/ImguiHelper.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Utility/Promise.hpp"

//------------------------------------------
// Helper
namespace {
ImVec4 colorFromIndex(std::size_t const i, std::size_t const n) {
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

struct Labels {
    std::string nameX;
    std::string nameY;
    std::string nameColour;
    std::string remove;

    explicit Labels(std::uint16_t const id) {
        nameX = "##PlotX_" + std::to_string(id);
        nameY = "##PlotY_" + std::to_string(id);
        nameColour = "##PlotColor_" + std::to_string(id);
        remove = "Remove##PlotRemove_" + std::to_string(id);
    }
};

struct PlotData {
    std::array<char, 256> bufX{};
    std::array<char, 256> bufY{};
    Nebulite::Interaction::Logic::Expression expressionX;
    Nebulite::Interaction::Logic::Expression expressionY;
    std::list<std::pair<double, double>> points;
    ImVec4 colour;
    Labels labels;

    PlotData(std::uint16_t const id, ImVec4 const& plotColour)
        : expressionX{"$({global:time.t})"}
        , expressionY{"$(sin({global:time.t}))"}
        , colour{plotColour}
        , labels{id}{
        std::strncpy(bufX.data(), expressionX.getFullExpression().c_str(), bufX.size() - 1);
        std::strncpy(bufY.data(), expressionY.getFullExpression().c_str(), bufY.size() - 1);
    }
};

struct Plots {
    std::uint16_t idCounter = 0;
    std::list<PlotData> plots;

    int pointsToKeep = 200;

    double lastUpdated = 0; // seconds
    static double constexpr updateInterval = 0.01; // seconds

    std::array<char, 256> bufXMin = {};
    std::array<char, 256> bufXMax = {};
    std::array<char, 256> bufYMin = {};
    std::array<char, 256> bufYMax = {};

    Nebulite::Interaction::Logic::Expression expressionXMin;
    Nebulite::Interaction::Logic::Expression expressionXMax;

    Nebulite::Interaction::Logic::Expression expressionYMin;
    Nebulite::Interaction::Logic::Expression expressionYMax;

    Plots() : expressionXMin("$({global:time.t} - 50)"), expressionXMax("$({global:time.t})"), expressionYMin("$(-2)"), expressionYMax("$(2)") {
        std::strncpy(bufXMin.data(), expressionXMin.getFullExpression().c_str(), 256);
        std::strncpy(bufXMax.data(), expressionXMax.getFullExpression().c_str(), 256);
        std::strncpy(bufYMin.data(), expressionYMin.getFullExpression().c_str(), 256);
        std::strncpy(bufYMax.data(), expressionYMax.getFullExpression().c_str(), 256);
    }

    bool validLimits() const {
        return expressionXMin.isReturnableAsDouble()
            && expressionXMax.isReturnableAsDouble()
            && expressionYMin.isReturnableAsDouble()
            && expressionYMax.isReturnableAsDouble();
    }

    void update(Nebulite::Interaction::ContextScope const& ctxScope) {
        if (lastUpdated + updateInterval < ImGui::GetTime()) {
            lastUpdated = ImGui::GetTime();
            for (auto& plot : plots) {
                if (plot.expressionX.isReturnableAsDouble() && plot.expressionY.isReturnableAsDouble()) {
                    double const x = plot.expressionX.evalAsDouble(ctxScope, Nebulite::Utility::Promise<&Nebulite::Interaction::Logic::Expression::isReturnableAsDouble>());
                    double const y = plot.expressionY.evalAsDouble(ctxScope, Nebulite::Utility::Promise<&Nebulite::Interaction::Logic::Expression::isReturnableAsDouble>());
                    plot.points.emplace_back(x, y);
                    while (plot.points.size() > static_cast<size_t>(pointsToKeep)) {
                        plot.points.pop_front();
                    }
                }
            }
        }
    }

    static void plotList(std::list<std::pair<double, double>> const& points, ImVec4 const& colour){
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

    void plot(Nebulite::Interaction::ContextScope const& ctxScope) {
        ImGui::Separator();
        if (ImPlot::BeginPlot("Plot", ImVec2(-1,0), ImPlotFlags_NoLegend)) {
            if (validLimits()) {
                double const xMin = expressionXMin.evalAsDouble(ctxScope, Nebulite::Utility::Promise<&Nebulite::Interaction::Logic::Expression::isReturnableAsDouble>());
                double const xMax = expressionXMax.evalAsDouble(ctxScope, Nebulite::Utility::Promise<&Nebulite::Interaction::Logic::Expression::isReturnableAsDouble>());
                double const yMin = expressionYMin.evalAsDouble(ctxScope, Nebulite::Utility::Promise<&Nebulite::Interaction::Logic::Expression::isReturnableAsDouble>());
                double const yMax = expressionYMax.evalAsDouble(ctxScope, Nebulite::Utility::Promise<&Nebulite::Interaction::Logic::Expression::isReturnableAsDouble>());
                ImPlot::SetupAxesLimits(xMin, xMax, yMin, yMax, ImPlotCond_Always);
            }

            for (auto& plot : plots) {
                plotList(plot.points, plot.colour);
            }
            ImPlot::EndPlot();
        }
    }
};

template<std::size_t N = 256>
bool imguiLinkedInput(std::array<char, N>& buf, Nebulite::Interaction::Logic::Expression& expr, char const* label) {
    static auto constexpr yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    static auto constexpr red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    bool const pushedColour = [&] {
        if (!expr.isReturnableAsDouble()) {
            ImGui::PushStyleColor(ImGuiCol_Text, red);
            return true;
        }
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
        }
        expr = Nebulite::Interaction::Logic::Expression{exprStr};
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

template<std::size_t N, typename F>
void imguiTable(char const* label, std::array<char const*, N> c, F&& f) {
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

[[maybe_unused]] void renderPlotTable(int const columns, std::list<PlotData>& availablePlots) {
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

} // namespace

//------------------------------------------
namespace Nebulite::Graphics {

void ImguiHelper::renderPlotViewer(Interaction::ContextScope const& ctxScope, std::string const& identifier){
    static std::unordered_map<std::string, Plots> plots;
    auto& plotMetaData = plots[identifier];
    auto& idCounter = plotMetaData.idCounter;
    auto& availablePlots = plotMetaData.plots;

    // Window
    imguiChild("PlotViewer", [&] {
        ImGui::Separator();

        // Add Plot
        ImGui::TextUnformatted("Available Plots:");
        ImGui::SameLine();
        static auto constexpr addPlotText = " + Add Plot";
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - ImGui::CalcTextSize(addPlotText).x - ImGui::GetStyle().FramePadding.x * 2.0f);
        if (ImGui::Button(addPlotText)) {
            availablePlots.emplace_back(idCounter++, colorFromIndex(availablePlots.size(), 10));
        }

        // Table of available plots
        imguiTable("AvailablePlotsTable", std::array{"X Expression", "Y Expression", "Colour", "Remove"}, [&] {
            for (auto it = availablePlots.begin(); it != availablePlots.end();) {
                auto& [bufX, bufY, x, y, points, colour, labels] = *it;
                ImGui::TableNextRow();

                // [X input]
                ImGui::TableNextColumn();
                if (imguiLinkedInput(bufX, x, labels.nameX.data())) {
                    points.clear(); // Clear points if expression changes
                }

                // [Y input]
                ImGui::TableNextColumn();
                if (imguiLinkedInput(bufY, y, labels.nameY.data())) {
                    points.clear(); // Clear points if expression changes
                }

                // [Colour input]
                ImGui::TableNextColumn();
                ImGui::ColorEdit4(labels.nameColour.c_str(), reinterpret_cast<float*>(&colour), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                // [Remove button]
                ImGui::TableNextColumn();
                if (ImGui::Button(labels.remove.c_str())) {
                    it = availablePlots.erase(it);
                } else {
                    ++it;
                }
            }
        });

        //------------------------------------------
        // Axis limits input + Points to keep

        ImGui::Separator();
        imguiTable("LimitsTable", std::array{"xMin Expression", "xMax Expression", "yMin Expression", "yMax Expression", "N"}, [&] {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            imguiLinkedInput(plotMetaData.bufXMin, plotMetaData.expressionXMin, "#xMin");
            ImGui::TableNextColumn();
            imguiLinkedInput(plotMetaData.bufXMax, plotMetaData.expressionXMax, "#xMax");
            ImGui::TableNextColumn();
            imguiLinkedInput(plotMetaData.bufYMin, plotMetaData.expressionYMin, "#yMin");
            ImGui::TableNextColumn();
            imguiLinkedInput(plotMetaData.bufYMax, plotMetaData.expressionYMax, "#yMax");
            ImGui::TableNextColumn();
            ImGui::InputInt("##PointsToKeep", &plotMetaData.pointsToKeep, 1, 10);
            plotMetaData.pointsToKeep = std::max(1, plotMetaData.pointsToKeep);
        });

        //------------------------------------------
        // Update and plot

        plotMetaData.update(ctxScope);
        plotMetaData.plot(ctxScope);
    });
}

} // namespace Nebulite::Graphics
