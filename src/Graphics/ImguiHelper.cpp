//------------------------------------------
// Includes

// External
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// External
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/GlobalSpace.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/KeyType.hpp"
#include "Graphics/ImguiHelper.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Module/Domain/Common/General.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/Ranges.hpp"
#include "Utility/Sort.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
// Helper functions

namespace {

size_t findFileSeparatorPositionOrFallback(std::string_view const str) {
    auto const result = std::ranges::find_last_if(str, [](char const& c) {
        return c == Nebulite::Utility::IO::FileManagement::preferredSeparator();
    });

    if (result.empty()) {
        return 0;
    }

    return static_cast<size_t>(std::ranges::distance(str.begin(), result.begin()));
}

void addRootCompletions(std::string_view const input, std::vector<std::string>& completions, Nebulite::Interaction::Execution::Domain const& domain) {
    auto args = Nebulite::Utility::StringHandler::parseQuotedArguments(input).args;
    if (args.empty() || input.ends_with(' ')) {
        args = {""};
    }
    std::string const& pattern = args.back();  // Get last argument, which is the one we want to complete

    auto rootCompletions = domain.findCompletions(pattern);
    completions.insert(completions.end(), rootCompletions.begin(), rootCompletions.end());
}

void addFileCompletions(std::string_view const input, std::vector<std::string>& completions) {
    auto args = Nebulite::Utility::StringHandler::parseQuotedArguments(input).args;
    if (args.empty()) {
        args.emplace_back(""); // If there are no arguments, we want to complete the first one, which is empty
    }

    // Separate inner from outer directory and get the actual input we need to complete
    std::string const& pattern = args.back();  // Get last argument, which is the one we want to complete
    std::size_t const startIndex = pattern.starts_with("./") ? 2 : 0; // If pattern starts with "./", we want to ignore that for file searching
    std::size_t const endIndex = findFileSeparatorPositionOrFallback(pattern.substr(startIndex)) + startIndex;
    auto const inputToComplete = pattern.substr(endIndex != startIndex ? endIndex + 1 : startIndex);
    auto const innerDir = std::string(pattern.substr(startIndex, endIndex - startIndex)) + Nebulite::Utility::IO::FileManagement::preferredSeparator();
    auto const directory = Nebulite::Utility::IO::FileManagement::CombinePaths(".", innerDir == "/" ? "" : innerDir);

    // Build list
    auto const list = Nebulite::Utility::IO::FileManagement::listContentInDirectory(directory)
        | std::views::filter([&](std::string const& fileOrDirectory) {
            return fileOrDirectory.starts_with(inputToComplete);
        })
        | std::views::transform([&](std::string const& fileOrDirectory) -> std::optional<std::string> {
            // Make sure to append a separator if it's a directory
            try {
                if (Nebulite::Utility::IO::FileManagement::isDirectory(directory + fileOrDirectory)) {
                    return fileOrDirectory + Nebulite::Utility::IO::FileManagement::preferredSeparator();
                }
                return fileOrDirectory;
            } catch (...) {
                return std::nullopt; // Something went wrong, skip this entry
            }
        })
        | std::views::filter([](std::optional<std::string> const& opt) { return opt.has_value(); })
        | Nebulite::Utility::Ranges::collectOptional;

    if (!list.has_value()) return; // Something went wrong, skip completions
    std::ranges::move(list.value(), std::back_inserter(completions));
}

void addJsonCompletions(std::string_view const input, std::vector<std::string>& completions, Nebulite::Data::JsonScope const& scope) {
    auto const split = Nebulite::Utility::StringHandler::split(input, ' ');
    if (split.empty()) return;
    auto const& potentialKey = split.back();
    auto const search = std::string(1, Nebulite::Data::JSON::SpecialCharacter::dot) + Nebulite::Data::JSON::SpecialCharacter::arrayClose;
    auto const parentMemberPos = potentialKey.find_last_of(search);
    auto parentMember = parentMemberPos == std::string::npos ? "" : potentialKey.substr(0, parentMemberPos+1);
    if (parentMember.ends_with(".")) parentMember = parentMember.substr(0, parentMemberPos);
    for (auto const& [member, key] : scope.listAvailableMembersAndKeys(scope.getRootScope().addMember(parentMember))) {
        auto const entry = Nebulite::Data::ScopedKey(parentMember).addMember(member).toString();
        if (!entry.starts_with(potentialKey)) continue;
        completions.push_back(entry);
    }
}

struct ConsoleState {
    std::string command;
    std::string draftCommand;
    std::size_t historyIndex = 0;
    Nebulite::Utility::IO::Capture* capture = nullptr;
    Nebulite::Interaction::Context* ctx = nullptr;
    Nebulite::Interaction::ContextScope* ctxScope = nullptr;
};

// If all completions start with the same prefix, the completions vector is modified:
// input: "ca", completions: ["car-wash", "car-insurance"] -> ["car-"]
bool checkCompletionsForCommonPrefix(std::string_view const input, std::vector<std::string>& completions) {
    if (completions.empty()) {
        return false;
    }
    auto const& first = completions.front();
    auto const matchingLength = [&] {
        auto enumeratedString = std::views::enumerate(first);
        auto const matchIterator = std::ranges::find_if(
            enumeratedString,
            [&](auto tuple) {
                auto const [i, c] = tuple;
                auto const idx = static_cast<std::size_t>(i);
                return std::ranges::any_of(completions, [&](std::string const& s) {
                    return idx >= s.size() || s[idx] != c;
                });
            }
        );
        if (matchIterator == enumeratedString.end()) {
            return first.size();
        }
        return static_cast<std::size_t>(std::get<0>(*matchIterator));
    }();
    if (auto const match = first.substr(0, matchingLength); !match.empty() && !input.ends_with(match)) {
        completions.clear();
        completions.push_back(match);
        return true;
    }
    return false;
}

void historyScrollingCallback(ImGuiInputTextCallbackData* data, ConsoleState* state) {
    auto const historySize = state->capture->getHistory().size();
    if (data->EventKey == ImGuiKey_UpArrow) {
        std::size_t newIndex = state->historyIndex;
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
            return; // Already at the end of history, nothing to do
        }
        std::size_t newIndex = state->historyIndex - 1;
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

void completionCallback(ImGuiInputTextCallbackData* data, ConsoleState const* state) {
    // Build completions list
    auto completions = state->ctx->self.findCompletions(state->command);
    if (completions.empty()) addRootCompletions(state->command, completions, state->ctx->self);
    addFileCompletions(state->command, completions);
    addJsonCompletions(state->command, completions, state->ctxScope->self);

    // Finish completions
    std::ranges::sort(completions, Nebulite::Utility::Sort::caseInsensitiveLess);
    auto v = std::ranges::unique(completions);
    completions.erase(v.begin(), v.end());
    auto const commonPrefixFound = completions.size() != 1 && checkCompletionsForCommonPrefix(state->command, completions);

    // Check completions
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

        // Insert additional whitespace under certain conditions:
        static auto endCharsToIgnore = {
            Nebulite::Utility::IO::FileManagement::preferredSeparator(), // Directory Path
            Nebulite::Data::JSON::SpecialCharacter::dot, // JSON indexing
            Nebulite::Data::JSON::SpecialCharacter::arrayClose // JSON array indexing
        };
        if (!commonPrefixFound && !toInsert.empty() && !std::ranges::any_of(endCharsToIgnore, [&](char const& c) { return toInsert.back() == c; })) {
            data->InsertChars(data->CursorPos, " ");
        }
    }
    else if (completions.size() > 1) {
        // TODO: determine console width in characters
        state->capture->log.println(Nebulite::Utility::StringHandler::createPaddedTable(completions, 80));
    }
}

// NOLINTNEXTLINE
int consoleInputCallback(ImGuiInputTextCallbackData* data) {
    auto* state = static_cast<ConsoleState*>(data->UserData);
    if (!state) return 0;

    // Should not happen
    if (!state->capture || !state->ctx || !state->ctxScope) return 0;

    // Check callback type...
    switch (data->EventFlag) {
    case ImGuiInputTextFlags_CallbackHistory:
        historyScrollingCallback(data, state);
        break;
    case ImGuiInputTextFlags_CallbackCompletion:
        completionCallback(data, state);
        break;
    default:
        break;
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
    renderJsonTreeNode(scope, scope.getRootScope());
    ImGui::End();
}

void ImguiHelper::align(DomainRenderingFlags::Alignment const& alignment) {
    ImGuiViewport const* const vp = ImGui::GetMainViewport();

    ImVec2 const vpPos  = vp->WorkPos;
    ImVec2 const vpSize = vp->WorkSize;

    auto const topPos    = ImVec2(vpPos.x, vpPos.y);
    auto const bottomPos = ImVec2(vpPos.x, vpPos.y + vpSize.y);
    auto const leftPos   = ImVec2(vpPos.x, vpPos.y);
    auto const rightPos  = ImVec2(vpPos.x + vpSize.x, vpPos.y);

    switch (alignment) {
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
    renderJsonTreeNode(scope, scope.getRootScope());
    ImGui::EndChild();
    ImGui::Columns(1); // Restore single column
    ImGui::End();
}

void ImguiHelper::renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKeyView const& root) {
    for (auto const& key : s.listAvailableKeys(root)) {
        std::string const rootPath = root.toString();
        std::string const fullPath = key.view().toString();
        std::string keyPath = fullPath;
        if (rootPath != fullPath) keyPath = fullPath.substr(rootPath.length());
        if (!keyPath.empty() && keyPath.front() == Data::JSON::SpecialCharacter::dot) keyPath.erase(0, 1);
        if (auto const type = s.memberType(key); type == Data::KeyType::object || type == Data::KeyType::array) {
            // use fullPath as the ID (first arg) and keyPath as the visible text (format)
            if (ImGui::TreeNode(fullPath.c_str(), "%s", keyPath.c_str())) {
                renderJsonTreeNode(s, key.view());
                ImGui::TreePop();
            }
        } else if (type == Data::KeyType::value) {
            if (auto const stringValue = s.get<std::string>(key); stringValue.has_value()) {
                ImGui::Text("%s : %s", keyPath.c_str(), stringValue.value().c_str());
            }
            else {
                ImGui::TextDisabled("%s : <err: failed to convert value to string>", keyPath.c_str());
            }
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
    for (const auto& [content, type, silent] : capture.getHistory()){
        if (!silent) continue; // Skip silent entries
        std::string contentFull;
        switch (type) {
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
            capture.appendToHistory(command, Utility::IO::HistoryLine::Type::Input);
            Global::instance().notifyEvent(domain.parseStr(__FUNCTION__ + std::string(" ") + command, ctx, ctxScope));
            command.clear();
            state.historyIndex = 0; // Reset history index after executing a command
        }
        ImGui::SetKeyboardFocusHere(-1);    // focus again
    }
}

} // namespace Nebulite::Graphics
