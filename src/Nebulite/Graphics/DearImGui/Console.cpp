//------------------------------------------
// Includes

// External
#include <algorithm>
#include <cstddef>
#include <cstdint> // NOLINT
#include <cstring> // NOLINT
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
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Graphics/DearImGui/Core.hpp"
#include "Nebulite/Graphics/DearImGui/Helper.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/Io/FileManagement.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/Sort.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
// Helper functions

namespace {

size_t findFileSeparatorPositionOrFallback(std::string_view const str) {
    auto const result = std::ranges::find_last_if(str, [](char const& c) {
        return c == Nebulite::Utility::Io::FileManagement::preferredSeparator();
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
    auto const innerDir = std::string(pattern.substr(startIndex, endIndex - startIndex)) + Nebulite::Utility::Io::FileManagement::preferredSeparator();
    auto const directory = Nebulite::Utility::Io::FileManagement::combinePaths(".", innerDir == "/" ? "" : innerDir);

    // Build list
    auto const list = Nebulite::Utility::Io::FileManagement::listContentInDirectory(directory)
        | std::views::filter([&](std::string const& fileOrDirectory) {
            return fileOrDirectory.starts_with(inputToComplete);
        })
        | std::views::transform([&](std::string const& fileOrDirectory) -> std::optional<std::string> {
            // Make sure to append a separator if it's a directory
            try {
                if (Nebulite::Utility::Io::FileManagement::isDirectory(directory + fileOrDirectory)) {
                    return fileOrDirectory + Nebulite::Utility::Io::FileManagement::preferredSeparator();
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
    auto const search = std::string(1, Nebulite::Data::Json::SpecialCharacter::dot) + Nebulite::Data::Json::SpecialCharacter::arrayClose;
    auto const parentMemberPos = potentialKey.find_last_of(search);
    auto parentMember = parentMemberPos == std::string::npos ? "" : potentialKey.substr(0, parentMemberPos+1);
    if (parentMember.ends_with(Nebulite::Data::Json::SpecialCharacter::dot)) parentMember = parentMember.substr(0, parentMemberPos);
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
    Nebulite::Utility::Io::Capture* capture = nullptr;
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
        auto enumeratedString = Nebulite::Utility::Ranges::enumerate(first);
        auto const matchIterator = std::ranges::find_if(
            enumeratedString,
            [&](auto tuple) {
                auto const [idx, c] = tuple;
                return std::ranges::any_of(completions, [&](auto const& s) {
                    return idx >= s.size() || s[idx] != c;
                });
            }
        );
        if (matchIterator == enumeratedString.end()) {
            return first.size();
        }
        return std::get<0>(*matchIterator);
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
            if (state->capture->getHistory().at(historySize - newIndex).type == Nebulite::Utility::Io::HistoryLine::Type::input) {
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
            if (state->capture->getHistory().at(historySize - newIndex).type == Nebulite::Utility::Io::HistoryLine::Type::input) {
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
        std::string const& cmd = state->command;

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
            Nebulite::Utility::Io::FileManagement::preferredSeparator(), // Directory Path
            Nebulite::Data::Json::SpecialCharacter::dot, // JSON indexing
            Nebulite::Data::Json::SpecialCharacter::arrayClose, // JSON array indexing
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
namespace Nebulite::Graphics::DearImGui {

void Helper::renderDomainViewerConsole(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name) {
    static auto constexpr function = __FUNCTION__;

    //------------------------------------------
    // Console output area:

    Core::imguiChild("#Console", [&] {
        //------------------------------------------
        // Console output area

        Core::imguiChild("ConsoleOutput", [&] {
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

        Core::imguiChild("ConsoleInput", [&] {
            // Store state for each console by name
            static std::unordered_map<std::string, ConsoleState> states;
            if (auto const it = states.find(name); it == states.end()) {
                // Initialize state for this console if it doesn't exist
                states[name] = ConsoleState();
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
            if (ImGui::InputText("##ConsoleInput", &command, flags, consoleInputCallback, &state)) {
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

} // namespace Nebulite::Graphics::DearImGui
