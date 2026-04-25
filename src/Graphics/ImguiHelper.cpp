//------------------------------------------
// Includes

// External
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Module/Domain/Common/General.hpp"
#include "Graphics/ImguiHelper.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/FileManagement.hpp"

//------------------------------------------

namespace {

size_t find_pos_or_fallback(std::string_view const& str) {
    auto const result = std::ranges::find_last_if(str, [](char const& c) {
        return c == Nebulite::Utility::IO::FileManagement::preferredSeparator();
    });

    if (result.empty()) {
        return 0;
    }

    return static_cast<size_t>(std::ranges::distance(str.begin(), result.begin()));
}

void addFileCompletions(std::string_view const& input, std::vector<std::string>& completions) {
    auto const args = Nebulite::Utility::StringHandler::parseQuotedArguments(input).args;
    if (args.empty()) {
        return;
    }

    // Separate inner from outer directory and get the actual input we need to complete
    std::string const& pattern = args.back();  // Get last argument, which is the one we want to complete
    size_t const startIndex = pattern.starts_with("./") ? 2 : 0; // If pattern starts with "./", we want to ignore that for file searching
    size_t const endIndex = find_pos_or_fallback(pattern.substr(startIndex)) + startIndex;
    auto const inputToComplete = pattern.substr(endIndex != startIndex ? endIndex + 1 : startIndex);
    auto const innerDir = std::string(pattern.substr(startIndex, endIndex - startIndex)) + Nebulite::Utility::IO::FileManagement::preferredSeparator();
    auto const directory = Nebulite::Utility::IO::FileManagement::CombinePaths(".", innerDir == "/" ? "" : innerDir);

    // Build list
    auto const list = Nebulite::Utility::IO::FileManagement::listFilesAndDirectoriesInPath(directory) | std::views::filter([&](std::string const& fileOrDirectory) {
        return fileOrDirectory.starts_with(inputToComplete);
    }) | std::views::transform([&](std::string const& fileOrDirectory) {
        if (Nebulite::Utility::IO::FileManagement::isDirectory(directory + fileOrDirectory)) {
            return fileOrDirectory + Nebulite::Utility::IO::FileManagement::preferredSeparator();
        }
        return fileOrDirectory;
    }) | std::ranges::to<std::vector>();
    std::ranges::move(list, std::back_inserter(completions));
    std::ranges::sort(completions, Nebulite::Utility::Sort::caseInsensitiveLess);
}

struct ConsoleState {
    std::string command;
    std::string draftCommand;
    size_t historyIndex = 0;
    Nebulite::Utility::IO::Capture* capture = nullptr;
    Nebulite::Interaction::Context* ctx = nullptr;
    Nebulite::Interaction::ContextScope* ctxScope = nullptr;
};

void checkCompletionsForCommonPrefix(std::string_view const& input, std::vector<std::string>& completions) {
    if (completions.empty()) {
        return;
    }
    const auto& first = completions.front();
    auto const mismatch_it = std::ranges::find_if(
    std::views::iota(std::size_t{0}, first.size()),
    [&](std::size_t const& i) {
        char const c = first[i];
        return std::ranges::any_of(completions, [&](const std::string& s) {
            return i >= s.size() || s[i] != c;
        });
    });

    if (std::string const match = first.substr(0, mismatch_it == std::views::iota(std::size_t{0}, first.size()).end() ? first.size() : *mismatch_it); !match.empty() && !input.ends_with(match)) {
        completions.clear();
        completions.push_back(match);
    }
}

// NOLINTNEXTLINE
int consoleInputCallback(ImGuiInputTextCallbackData* data) {
    auto* state = static_cast<ConsoleState*>(data->UserData);
    if (!state) return 0;

    // Should not happen
    if (!state->capture || !state->ctx || !state->ctxScope) return 0;

    // Check callback type...
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) { // History scrolling
        auto const historySize = state->capture->getHistory().size();

        if (data->EventKey == ImGuiKey_UpArrow) {
            size_t newIndex = state->historyIndex;
            if (state->historyIndex == 0) {
                state->draftCommand = state->command; // Save current command as draft if we are at the start of history
            }

            while (newIndex < state->capture->getHistory().size() - 1) {
                newIndex++;
                if (state->capture->getHistory().at(historySize - newIndex).type == Nebulite::Utility::IO::HistoryLine::Type::Input) {
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
                if (state->capture->getHistory().at(historySize - newIndex).type == Nebulite::Utility::IO::HistoryLine::Type::Input) {
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
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) { // Tab-Autocomplete
        auto completions = state->ctx->self.findCompletions(state->command);
        addFileCompletions(state->command, completions);
        checkCompletionsForCommonPrefix(state->command, completions);
        if (completions.size() == 1) {
            auto const& toInsert = completions.front();
            const std::string& cmd = state->command;

            // Find overlap suffix: the part at the end of the current command that matches the beginning of the completion.
            // e.g.: typed is fooBar, fooBarBaz is complete -> remove fooBar and insert full complete string.
            int overlap = 0;
            auto const maxCheck = static_cast<int>(std::min(cmd.size(), toInsert.size()));
            for (int i = 1; i <= maxCheck; ++i) {
                if (auto const idx = static_cast<size_t>(i); cmd.compare(cmd.size() - idx, idx, toInsert, 0, idx) == 0) {
                    overlap = i;
                }
            }

            // delete only the overlapping suffix and insert full completion
            data->DeleteChars(data->CursorPos - overlap, overlap);
            data->InsertChars(data->CursorPos, toInsert.c_str());
        }
        else if (completions.size() > 1) {
            state->capture->log.println(Nebulite::Utility::StringHandler::createPaddedTable(completions));
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

// TODO: Optional resizing (e.g. if aligned to bottom, allow user to resize height but keep aligned to bottom)
namespace {

void align(ImguiHelper::DomainRenderingFlags::Alignment const& alignment) {
    ImGuiViewport const* const vp = ImGui::GetMainViewport();

    ImVec2 const vpPos  = vp->WorkPos;
    ImVec2 const vpSize = vp->WorkSize;

    auto const topPos    = ImVec2(vpPos.x, vpPos.y);
    auto const bottomPos = ImVec2(vpPos.x, vpPos.y + vpSize.y);
    auto const leftPos   = ImVec2(vpPos.x, vpPos.y);
    auto const rightPos  = ImVec2(vpPos.x + vpSize.x, vpPos.y);

    switch (alignment) {
    case ImguiHelper::DomainRenderingFlags::Alignment::TOP:
        ImGui::SetNextWindowPos(topPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x, vpSize.y * 0.5f),
            ImGuiCond_Always
        );
        break;
    case ImguiHelper::DomainRenderingFlags::Alignment::BOTTOM:
        ImGui::SetNextWindowPos(bottomPos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x, vpSize.y * 0.5f),
            ImGuiCond_Always
        );
        break;
    case ImguiHelper::DomainRenderingFlags::Alignment::LEFT:
        ImGui::SetNextWindowPos(leftPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x * 0.5f, vpSize.y),
            ImGuiCond_Always
        );
        break;
    case ImguiHelper::DomainRenderingFlags::Alignment::RIGHT:
        ImGui::SetNextWindowPos(rightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x * 0.5f, vpSize.y),
            ImGuiCond_Always
        );
        break;
    case ImguiHelper::DomainRenderingFlags::Alignment::NONE:
        break;
    default:
        std::unreachable();
    }
}

} // namespace

void ImguiHelper::renderDomain(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::IO::Capture& capture, std::string const& name, DomainRenderingFlags const& flags) {
    auto const& domain = ctx.self;
    auto const& scope = ctxScope.self;
    std::string const additionalIdentifier = !domain.capture.hasParent() ? "GLOBAL" : "";
    std::string const windowName = "Nebulite Domain Interface - " + name + "###DomainViewer_" + name + "_" + std::to_string(domain.getId()) + "_" + additionalIdentifier;

    // Sizing and alignment
    if (flags.windowPos.has_value()) {
        ImGui::SetNextWindowPos(flags.windowPos.value(), ImGuiCond_Always);
    }
    if (flags.windowSize.has_value()) {
        ImGui::SetNextWindowSize(flags.windowSize.value(), ImGuiCond_Always);
    }
    if (flags.windowAlignment.has_value()) {
        align(flags.windowAlignment.value());
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
            if (auto const event = domain.parseStr(__FUNCTION__ + std::string(" ") + Module::Domain::Common::General::imguiView_Disable, ctx, ctxScope); event != Constants::Event::Success) {
                capture.warning.println("Error disabling ImGui view for domain " + name);
            }
        }
    }

    // Console + JSON viewer in two columns
    ImGui::Columns(2, nullptr, true);
    ImGui::BeginChild("DomainConsole", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    renderDomainConsole(ctx, ctxScope, capture, name);
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
        if (!keyPath.empty() && keyPath.front() == Data::JSON::SpecialCharacter::dot) keyPath.erase(0, 1);
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

void ImguiHelper::renderDomainConsole(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::IO::Capture& capture, std::string const& name) {
    auto const& domain = ctx.self;

    // Console output area
    ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

    ImGui::PushTextWrapPos(0.0f); // wrap at window/child width
    for (const auto& [content, lineType] : capture.getHistory()){
        std::string contentFull;
        switch (lineType) {
            case Utility::IO::HistoryLine::Type::Info:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // white
                break;
            case Utility::IO::HistoryLine::Type::Warning:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 165.0f / 265.0f, 0.0f, 1.0f)); // orange
                break;
            case Utility::IO::HistoryLine::Type::Error:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // red
                break;
            case Utility::IO::HistoryLine::Type::Input:
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
    state.ctx = &ctx;
    state.ctxScope = &ctxScope;
    std::string& command = state.command; // Get command buffer for this console
    command.reserve(256); // Pre-allocate to avoid reallocations during typing

    if (ImGui::InputText("##ConsoleInput", &command, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion, consoleInputCallback, &state)) {
        if (!command.empty()){
            capture.appendInput(command);
            Global::instance().notifyEvent(domain.parseStr(__FUNCTION__ + std::string(" ") + command, ctx, ctxScope));
            command.clear();
            state.historyIndex = 0; // Reset history index after executing a command
        }
        ImGui::SetKeyboardFocusHere(-1);    // focus again
    }
}

} // namespace Nebulite::Graphics
