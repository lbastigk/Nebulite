//------------------------------------------
// Includes

// External
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Common/General.hpp"
#include "Graphics/ImguiHelper.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Nebulite.hpp"

//------------------------------------------

namespace {
struct ConsoleState {
    std::string command;
    std::string draftCommand;
    size_t historyIndex = 0;
    Nebulite::Utility::Capture* capture = nullptr;
}; // namespace

// NOLINTNEXTLINE
int consoleInputCallback(ImGuiInputTextCallbackData* data) {
    auto* state = static_cast<ConsoleState*>(data->UserData);
    if (!state) return 0;

    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
        auto const historySize = state->capture->getHistory().size();

        // Determine if up or down arrow was pressed
        if (data->EventKey == ImGuiKey_UpArrow) {
            size_t newIndex = state->historyIndex;
            if (state->historyIndex == 0) {
                state->draftCommand = state->command; // Save current command as draft if we are at the start of history
            }

            while (newIndex < state->capture->getHistory().size() - 1) {
                newIndex++;
                if (state->capture->getHistory().at(historySize - newIndex).type == Nebulite::Utility::HistoryLine::Type::Input) {
                    state->historyIndex = newIndex;
                    state->command = state->capture->getHistory().at(historySize-state->historyIndex).content; // Load command from history
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, state->command.c_str());
                    break;
                }
            }
        }
        else if (data->EventKey == ImGuiKey_DownArrow) {
            if (state->historyIndex == 0) {
                return 0; // Already at the end of history, nothing to do
            }
            size_t newIndex = state->historyIndex - 1;
            while (newIndex > 0) {
                if (state->capture->getHistory().at(historySize - newIndex).type == Nebulite::Utility::HistoryLine::Type::Input) {
                    state->historyIndex = newIndex;
                    break;
                }
                newIndex--;
            }
            if (newIndex == 0) {
                state->historyIndex = newIndex;
                state->command = state->draftCommand; // Restore draft command if we go back to the start
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, state->command.c_str());
            }
            else {
                state->command = state->capture->getHistory().at(historySize - state->historyIndex).content; // Load command from history
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, state->command.c_str());
            }
        }
    }
    return 0;
}
} // namespace

//------------------------------------------
namespace Nebulite::Graphics {

bool ImguiHelper::checkImguiInitialized() {
    return ImGui::GetCurrentContext() != nullptr;
}

bool ImguiHelper::checkImguiReadyForRendering() {
    return checkImguiInitialized() && ImGui::GetFrameCount() > 0;
}

void ImguiHelper::renderJsonScope(Data::JsonScope const& scope, std::string const& name) {
    ImGui::Begin(name.c_str());
    renderJsonTreeNode(scope, scope.getRootScope().toScopedKey());
    ImGui::End();
}

void ImguiHelper::renderDomain(Interaction::Execution::Domain& domain, Utility::Capture& capture, Data::JsonScope const& scope, std::string const& name, DomainRenderingFlags const& flags) {
    std::string const additionalIdentifier = !domain.capture.hasParent() ? "GLOBAL" : "";
    std::string const windowName = "Nebulite Domain Interface - " + name + "###DomainViewer_" + name + "_" + std::to_string(domain.getId()) + "_" + additionalIdentifier;

    // Alignment and sizing

    if (flags.windowPos.has_value()) {
        ImGui::SetNextWindowPos(flags.windowPos.value(), ImGuiCond_Always);
    }
    if (flags.windowSize.has_value()) {
        ImGui::SetNextWindowSize(flags.windowSize.value(), ImGuiCond_Always);
    }

    // TODO: Optional resizing (e.g. if aligned to bottom, allow user to resize height but keep aligned to bottom)
    if (flags.windowAlignment.has_value()) {
        ImGuiViewport const* const vp = ImGui::GetMainViewport();

        ImVec2 const vpPos  = vp->WorkPos;
        ImVec2 const vpSize = vp->WorkSize;

        auto const topPos    = ImVec2(vpPos.x, vpPos.y);
        auto const bottomPos = ImVec2(vpPos.x, vpPos.y + vpSize.y);
        auto const leftPos   = ImVec2(vpPos.x, vpPos.y);
        auto const rightPos  = ImVec2(vpPos.x + vpSize.x, vpPos.y);

        switch (flags.windowAlignment.value()) {
            case DomainRenderingFlags::Alignment::TOP:
                ImGui::SetNextWindowPos(topPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowSize(
                    ImVec2(vpSize.x, vpSize.y * 0.5f),
                    ImGuiCond_Always
                );
                break;
            case DomainRenderingFlags::Alignment::BOTTOM:
                ImGui::SetNextWindowPos(bottomPos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));
                ImGui::SetNextWindowSize(
                    ImVec2(vpSize.x, vpSize.y * 0.5f),
                    ImGuiCond_Always
                );
                break;
            case DomainRenderingFlags::Alignment::LEFT:
                ImGui::SetNextWindowPos(leftPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowSize(
                    ImVec2(vpSize.x * 0.5f, vpSize.y),
                    ImGuiCond_Always
                );
                break;
            case DomainRenderingFlags::Alignment::RIGHT:
                ImGui::SetNextWindowPos(rightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
                ImGui::SetNextWindowSize(
                    ImVec2(vpSize.x * 0.5f, vpSize.y),
                    ImGuiCond_Always
                );
                break;
            case DomainRenderingFlags::Alignment::NONE:
                break;
            default:
                std::unreachable();
        }
    }

    ImGui::Begin(windowName.c_str());

    // Header row
    ImGui::TextUnformatted(name.c_str());
    ImGui::SameLine();

    // Optional close button
    if (flags.showCloseButton) {
        // Right-align close button in the available content region
        float const buttonWidth = ImGui::CalcTextSize("Close").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float const cursorX = ImGui::GetCursorPosX();
        float const availX = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(cursorX + availX - buttonWidth);

        // Unique ID per console to avoid collisions
        std::string const closeId = "Close##DomainConsoleClose_" + name;
        if (ImGui::Button(closeId.c_str())) {
            // Instead of closing the window, we disable the ImGui view for this domain, allowing us to reopen it later without losing the capture and scope state
            if (auto const event = domain.parseStr(__FUNCTION__ + std::string(" ") + DomainModule::Common::General::imguiView_Disable); event != Constants::Event::Success) {
                capture.warning.println("Error disabling ImGui view for domain " + name);
            }
        }
    }

    // Console + JSON viewer in two columns
    ImGui::Columns(2, nullptr, true);
    ImGui::BeginChild("DomainConsole", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    renderDomainConsole(domain, capture, name);
    ImGui::EndChild();
    ImGui::NextColumn();
    ImGui::BeginChild("JsonScopeViewer", ImVec2(0, 0), true);
    renderJsonTreeNode(scope, scope.getRootScope().toScopedKey());
    ImGui::EndChild();
    ImGui::Columns(1); // Restore single column
    ImGui::End();
}

void ImguiHelper::renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKey const& root) {
    for (auto const& key : s.listAvailableKeys(root)) {
        std::string const rootPath = root.view().toString();
        std::string const fullPath = key.view().toString(); // stable ID
        std::string keyPath = fullPath;                   // visible label
        if (rootPath != fullPath) keyPath = fullPath.substr(rootPath.length());
        if (!keyPath.empty() && keyPath.front() == '.') keyPath.erase(0, 1);
        if (auto const type = s.memberType(key); type == Data::KeyType::object || type == Data::KeyType::array) {
            // use fullPath as the ID (first arg) and keyPath as the visible text (format)
            if (ImGui::TreeNode(fullPath.c_str(), "%s", keyPath.c_str())) {
                renderJsonTreeNode(s, key);
                ImGui::TreePop();
            }
        } else if (type == Data::KeyType::value) {
            // fetch value every frame and display
            std::string const valueStr = s.get<std::string>(key).value_or("<unavailable>");
            ImGui::Text("%s : %s", keyPath.c_str(), valueStr.c_str());
        } else {
            ImGui::TextDisabled("%s : null", keyPath.c_str());
        }
    }
}

void ImguiHelper::renderDomainConsole(Interaction::Execution::Domain& domain, Utility::Capture& capture, std::string const& name) {
    // Console output area
    ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

    ImGui::PushTextWrapPos(0.0f); // wrap at window/child width
    for (const auto& [content, lineType] : capture.getHistory()){
        std::string contentFull;
        switch (lineType) {
            case Utility::HistoryLine::Type::Info:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // white
                break;
            case Utility::HistoryLine::Type::Warning:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 165.0f / 265.0f, 0.0f, 1.0f)); // orange
                break;
            case Utility::HistoryLine::Type::Error:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // red
                break;
            case Utility::HistoryLine::Type::Input:
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
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    // Command input
    ImGui::Separator();

    static std::unordered_map<std::string, ConsoleState> states; // Store state for each console by name
    if (auto const it = states.find(name); it == states.end()) {
        // Initialize state for this console if it doesn't exist
        states[name] = ConsoleState();
    }
    auto& state = states.find(name)->second; // Get iterator again after potential insertion
    state.capture = &capture; // Set for history scrolling
    std::string& command = state.command; // Get command buffer for this console
    command.reserve(256); // Pre-allocate to avoid reallocations during typing

    if (ImGui::InputText("##ConsoleInput", &command, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory, consoleInputCallback, &state)) {
        if (!command.empty()){
            capture.appendInput(command);
            Global::instance().notifyEvent(domain.parseStr(__FUNCTION__ + std::string(" ") + command));
            command.clear();
            state.historyIndex = 0; // Reset history index after executing a command
        }
        ImGui::SetKeyboardFocusHere(-1);    // focus again
    }
}

} // namespace Nebulite::Graphics
