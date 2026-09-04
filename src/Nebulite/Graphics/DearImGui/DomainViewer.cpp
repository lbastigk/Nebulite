//------------------------------------------
// Includes

// External
#include <algorithm>
#include <array>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

// External
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Graphics/DearImGui/Align.hpp"
#include "Nebulite/Graphics/DearImGui/Color.hpp"
#include "Nebulite/Graphics/DearImGui/Console.hpp"
#include "Nebulite/Graphics/DearImGui/Core.hpp"
#include "Nebulite/Graphics/DearImGui/DomainViewer.hpp"
#include "Nebulite/Graphics/DearImGui/Json.hpp"
#include "Nebulite/Graphics/DearImGui/Plot.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Domain/Common/General.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::DomainViewer {

namespace {
/**
 * @enum FieldState
 * @brief State of each Field
 */
enum class FieldState : bool {
    visible,
    minimized,
};

/**
 * @brief Layout state of the domain viewer
 */
struct ViewerLayout {
    FieldState console = FieldState::visible;
    FieldState json = FieldState::visible;
    FieldState plot = FieldState::minimized; // Default to minimized
    static auto constexpr count = 3;
};
static_assert(sizeof(ViewerLayout) / sizeof(FieldState) == ViewerLayout::count, "Please update the count of ViewerLayout");

/**
 * @brief We map each Field in ViewerLayout to its title and content to render.
 */
struct FieldData {
    std::string title;
    FieldState& state;
    std::function<void()> renderFunction;
};

/**
 * @brief Setup ImGui for the next window based on the provided flags
 * @param flags The flags to consider
 */
void domainViewerSetup(RenderingFlags const& flags) {
    if (flags.windowPos.has_value()) {
        ImGui::SetNextWindowPos(flags.windowPos.value(), ImGuiCond_Always);
    }
    if (flags.windowSize.has_value()) {
        ImGui::SetNextWindowSize(flags.windowSize.value(), ImGuiCond_Always);
    }
    if (flags.windowAlignment.has_value()) {
        alignWindow(flags.windowAlignment.value());
    }
}

/**
 * @brief Renders the header of the domain viewer. With minimize tray and optional close button
 * @param flags The flags to consider
 * @param fields All available fields
 * @param windowName The name of the window
 * @param ctx The context of the window
 * @param ctxScope The context scope of the window
 * @param capture A capture instance to direct logging/warnings/errors to
 */
void renderDomainViewerHeader(RenderingFlags const& flags, std::array<FieldData, ViewerLayout::count>& fields, std::string const& windowName, Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture) {
    // Header row: Minimized fields, close button (optional)
    ImGui::SameLine();

    // Show minimized fields
    // Only show info if at least one field is minimized
    if (std::ranges::any_of(fields, [](FieldData const& field) { return field.state == FieldState::minimized; })) {
        ImGui::TextUnformatted("Minimized:");
        for (auto& field : fields) {
            if (field.state == FieldState::minimized) {
                ImGui::SameLine();
                if (ImGui::Button(field.title.c_str())) {
                    field.state = FieldState::visible;
                }
            }
        }
    }

    // Close button
    if (flags.showCloseButton) {
        ImGui::SameLine();
        // Right-align close button in the available content region
        float const buttonWidth = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float const cursorX = ImGui::GetCursorPosX();
        float const availX = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(cursorX + availX - buttonWidth);

        // Unique ID per console to avoid collisions
        std::string const closeId = "Close##DomainConsoleClose_" + windowName;
        if (auto const& domain = ctx.self; ImGui::Button(closeId.c_str())) {
            // Instead of closing the window, we disable the ImGui view for this domain, allowing us to reopen it later without losing the capture and scope state
            if (auto const event = domain.parseStr(__FUNCTION__ + std::string(" ") + Module::Domain::Common::General::imguiViewDisable, ctx, ctxScope); event != Constants::Event::success) {
                capture.warning.println("Error disabling ImGui view for domain " + windowName);
            }
        }
    }
}

/**
 * @brief Renders a given field of a domain viewer
 * @param id The id (is incremented per call)
 * @param title The title of the tile
 * @param state The State of the tile
 * @param content The content to render
 */
void renderViewerField(int& id, std::string const& title, FieldState& state, std::function<void()> const& content) {
    // Begin Child
    ImGui::PushID(id++);
    Core::renderChild(title.c_str(), [&] {
        // Header
        ImGui::TextUnformatted(title.c_str());
        ImGui::SameLine();
        Align::setCursorPosXForRightAlignedButton("Minimize");
        if (ImGui::Button("Minimize")) {
            state = FieldState::minimized;
        }

        // Content
        ImGui::Separator();
        content();
    });
    ImGui::PopID();
}
} // namespace

void render(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name, RenderingFlags const& flags) {
    auto const& scope = ctxScope.self;
    std::string const additionalIdentifier = !ctx.self.capture.hasParent() ? "GLOBAL" : "";
    std::string const identifier = name + "_" + std::to_string(ctx.self.getId()) + "_" + additionalIdentifier;
    std::string const windowName = "Nebulite Domain Interface - " + name;
    std::string const windowIdentifier = windowName + "###DomainViewer_" + identifier;

    // Viewer layout storage
    static std::unordered_map<std::string, ViewerLayout> layouts;

    // Available fields, their name and their function
    auto& [console, json, plot] = layouts[identifier];
    std::array fields = {
        FieldData{
            .title="Console",
            .state=console,
            .renderFunction=[&] {
                renderConsoleField(ctx, ctxScope, capture, name);
            },
        },
        FieldData{
            .title="JSON",
            .state=json,
            .renderFunction=[&] {
                renderJsonField(scope, scope.getRootScope());
            },
        },
        FieldData{
            .title="Plot",
            .state=plot,
            .renderFunction=[&] {
                renderPlotField(ctxScope, identifier);
            },
        },
    };
    static_assert(fields.size() == ViewerLayout::count, "Please update the field render logic to reflect the changes in ViewerLayout.");

    // Create window
    domainViewerSetup(flags);
    ImGui::Begin(windowIdentifier.c_str());
    renderDomainViewerHeader(flags, fields, windowName, ctx, ctxScope, capture);
    ImGui::Separator();

    // Render visible fields
    int const visibleCount = std::ranges::fold_left(fields, 0, [](int const count, FieldData const& field) {
        return count + (field.state == FieldState::visible ? 1 : 0);
    });
    if (visibleCount > 0) {
        if (ImGui::BeginTable("Viewers", visibleCount, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders)) {
            int columnId = 0;
            for (auto& [title, state, renderFunction] : fields) {
                if (state == FieldState::visible) {
                    ImGui::TableNextColumn();
                    renderViewerField(columnId, title, state, renderFunction);
                }
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void renderConsoleField(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name) {
    static auto constexpr function = __FUNCTION__;

    //------------------------------------------
    // Console output area:

    Core::renderChild("#Console", [&] {
        //------------------------------------------
        // Console output area

        Core::renderChild("ConsoleOutput", [&] {
            ImGui::PushTextWrapPos(0.0f); // wrap at window/child width
            for (auto const& [content, type] : capture.getHistory()){
                std::string contentFull;
                switch (type) {
                case Utility::Io::HistoryLine::Type::info:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // white
                    break;
                case Utility::Io::HistoryLine::Type::warning:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 165.0f / 265.0f, 0.0f, 1.0f)); // orange
                    break;
                case Utility::Io::HistoryLine::Type::error:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // red
                    break;
                case Utility::Io::HistoryLine::Type::input:
                    contentFull = "> ";
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f)); // grey
                    break;
                default:
                    std::unreachable();
                }
                contentFull += content;
                ImGui::TextUnformatted(contentFull.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::PopTextWrapPos();

            // Auto-scroll
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
        }, ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders);

        //------------------------------------------
        // Console input area

        Core::renderChild("ConsoleInput", [&] {
            // Store state for each console by name
            static std::unordered_map<std::string, Console::ConsoleState> states;
            if (auto const it = states.find(name); it == states.end()) {
                // Initialize state for this console if it doesn't exist
                states[name] = Console::ConsoleState();
            }

            // Set state
            auto& state = states.find(name)->second; // Get iterator again after potential insertion
            state.capture = &capture; // Set for history scrolling
            state.ctx = &ctx;
            state.ctxScope = &ctxScope;
            std::string& command = state.command; // Get command buffer for this console
            command.reserve(256); // Pre-allocate to avoid reallocations during typing

            // Flags are seen as unsigned but guaranteed to be >= 0, so we can or them together
            static auto constexpr flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion; // NOLINT
            if (ImGui::InputText("##ConsoleInput", &command, flags, Console::consoleInputCallback, &state)) {
                if (!command.empty()){
                    capture.appendToHistory(command, Utility::Io::HistoryLine::Type::input);
                    Global::instance().notifyEvent(ctx.self.parseStr(function + std::string(" ") + command, ctx, ctxScope));
                    command.clear();
                    state.historyIndex = 0; // Reset history index after executing a command
                }
                ImGui::SetKeyboardFocusHere(-1);    // focus again
            }
        }, ImVec2(0, 0), ImGuiChildFlags_None);
    }, ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_None);
}

void renderJsonField(Data::JsonScope const& scope, Data::ScopedKeyView const& root){
    Core::renderChild("JsonViewer", [&] {
        Json::renderScope(scope, root);
    });
}

void renderPlotField(Interaction::ContextScope const& ctxScope, std::string const& identifier){
    static std::unordered_map<std::string, Plot::Plots> plots;

    // Cleanup old plots
    std::erase_if(plots, [](auto const& pair) {
        return pair.second.lastUpdated + Plot::Plots::removeInterval < ImGui::GetTime();
    });

    // Get plots for the given identifier
    auto& plotMetaData = plots[identifier];
    auto& availablePlots = plotMetaData.plots;

    // Window
    Core::renderChild("PlotViewer", [&] {
        ImGui::Separator();

        // Add Plot
        ImGui::TextUnformatted("Available Plots:");
        ImGui::SameLine();
        static auto constexpr addPlotText = " + Add Plot";
        Align::setCursorPosXForRightAlignedButton(addPlotText);
        if (ImGui::Button(addPlotText)) {
            availablePlots.emplace_back(plotMetaData.idCounter++, Color::splitColorWheel(availablePlots.size(), 10));
        }

        // Table of available plots
        Core::renderTable("AvailablePlotsTable", std::array{"X Expression", "Y Expression", "Colour", "Remove"}, [&] {
            for (auto it = availablePlots.begin(); it != availablePlots.end();) {
                auto& [bufX, bufY, x, y, points, colour, labels] = *it;
                ImGui::TableNextRow();

                // [X input]
                ImGui::TableNextColumn();
                if (Plot::expressionInput(bufX, x, labels.nameX.data())) {
                    points.clear(); // Clear points if expression changes
                }

                // [Y input]
                ImGui::TableNextColumn();
                if (Plot::expressionInput(bufY, y, labels.nameY.data())) {
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
        Core::renderTable("LimitsTable", std::array{"xMin Expression", "xMax Expression", "yMin Expression", "yMax Expression", "N"}, [&] {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            Plot::expressionInput(plotMetaData.bufXMin, plotMetaData.expressionXMin, "#xMin");
            ImGui::TableNextColumn();
            Plot::expressionInput(plotMetaData.bufXMax, plotMetaData.expressionXMax, "#xMax");
            ImGui::TableNextColumn();
            Plot::expressionInput(plotMetaData.bufYMin, plotMetaData.expressionYMin, "#yMin");
            ImGui::TableNextColumn();
            Plot::expressionInput(plotMetaData.bufYMax, plotMetaData.expressionYMax, "#yMax");
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

} // namespace Nebulite::Graphics::DearImGui::DomainViewer
