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
#include <vector>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Graphics/DearImGui/Console.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/Io/FileManagement.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/Sort.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Console {

size_t findFileSeparatorPositionOrFallback(std::string_view const str) {
    auto const result = std::ranges::find_last_if(str, [](char const& c) {
        return c == Utility::Io::FileManagement::preferredSeparator();
    });

    if (result.empty()) {
        return 0;
    }

    return static_cast<size_t>(std::ranges::distance(str.begin(), result.begin()));
}

void addRootCompletions(std::string_view const input, std::vector<std::string>& completions, Interaction::Execution::Domain const& domain) {
    auto args = Utility::StringHandler::parseQuotedArguments(input).args;
    if (args.empty() || input.ends_with(' ')) {
        args = {""};
    }
    std::string const& pattern = args.back();  // Get last argument, which is the one we want to complete

    auto rootCompletions = domain.findCompletions(pattern);
    completions.insert(completions.end(), rootCompletions.begin(), rootCompletions.end());
}

void addFileCompletions(std::string_view const input, std::vector<std::string>& completions) {
    auto args = Utility::StringHandler::parseQuotedArguments(input).args;
    if (args.empty()) {
        args.emplace_back(""); // If there are no arguments, we want to complete the first one, which is empty
    }

    // Separate inner from outer directory and get the actual input we need to complete
    std::string const& pattern = args.back();  // Get last argument, which is the one we want to complete
    std::size_t const startIndex = pattern.starts_with("./") ? 2 : 0; // If pattern starts with "./", we want to ignore that for file searching
    std::size_t const endIndex = findFileSeparatorPositionOrFallback(pattern.substr(startIndex)) + startIndex;
    auto const inputToComplete = pattern.substr(endIndex != startIndex ? endIndex + 1 : startIndex);
    auto const innerDir = std::string(pattern.substr(startIndex, endIndex - startIndex)) + Utility::Io::FileManagement::preferredSeparator();
    auto const directory = Utility::Io::FileManagement::combinePaths(".", innerDir == "/" ? "" : innerDir);

    // Build list
    auto const list = Utility::Io::FileManagement::listContentInDirectory(directory)
        | std::views::filter([&](std::string const& fileOrDirectory) {
            return fileOrDirectory.starts_with(inputToComplete);
        })
        | std::views::transform([&](std::string const& fileOrDirectory) -> std::optional<std::string> {
            // Make sure to append a separator if it's a directory
            try {
                if (Utility::Io::FileManagement::isDirectory(directory + fileOrDirectory)) {
                    return fileOrDirectory + Utility::Io::FileManagement::preferredSeparator();
                }
                return fileOrDirectory;
            } catch (...) {
                return std::nullopt; // Something went wrong, skip this entry
            }
        })
        | std::views::filter([](std::optional<std::string> const& opt) { return opt.has_value(); })
        | Utility::Ranges::collectOptional;

    if (!list.has_value()) return; // Something went wrong, skip completions
    std::ranges::move(list.value(), std::back_inserter(completions));
}

void addJsonCompletions(std::string_view const input, std::vector<std::string>& completions, Data::JsonScope const& scope) {
    auto const split = Utility::StringHandler::split(input, ' ');
    if (split.empty()) return;
    auto const& potentialKey = split.back();
    auto const search = std::string(1, Data::Json::SpecialCharacter::dot) + Data::Json::SpecialCharacter::arrayClose;
    auto const parentMemberPos = potentialKey.find_last_of(search);
    auto parentMember = parentMemberPos == std::string::npos ? "" : potentialKey.substr(0, parentMemberPos+1);
    if (parentMember.ends_with(Data::Json::SpecialCharacter::dot)) parentMember = parentMember.substr(0, parentMemberPos);
    for (auto const& [member, key] : scope.listAvailableMembersAndKeys(scope.getRootScope().addMember(parentMember))) {
        auto const entry = Data::ScopedKey(parentMember).addMember(member).toString();
        if (!entry.starts_with(potentialKey)) continue;
        completions.push_back(entry);
    }
}

bool checkCompletionsForCommonPrefix(std::string_view const input, std::vector<std::string>& completions) {
    if (completions.empty()) {
        return false;
    }
    auto const& first = completions.front();
    auto const matchingLength = [&] {
        auto enumeratedString = Utility::Ranges::enumerate(first);
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
            if (state->capture->getHistory().at(historySize - newIndex).type == Utility::Io::HistoryLine::Type::input) {
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
            if (state->capture->getHistory().at(historySize - newIndex).type == Utility::Io::HistoryLine::Type::input) {
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
    std::ranges::sort(completions, Utility::Sort::caseInsensitiveLess);
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
            Utility::Io::FileManagement::preferredSeparator(), // Directory Path
            Data::Json::SpecialCharacter::dot, // JSON indexing
            Data::Json::SpecialCharacter::arrayClose, // JSON array indexing
        };
        if (!commonPrefixFound && !toInsert.empty() && !std::ranges::any_of(endCharsToIgnore, [&](char const& c) { return toInsert.back() == c; })) {
            data->InsertChars(data->CursorPos, " ");
        }
    }
    else if (completions.size() > 1) {
        // TODO: determine console width in characters
        state->capture->log.println(Utility::StringHandler::createPaddedTable(completions, 80));
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

} // namespace Nebulite::Graphics::DearImGui::Console
