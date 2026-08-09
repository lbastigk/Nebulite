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
#include <iterator>
#include <list>
#include <numeric>
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
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/ImguiHelper.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Domain/Common/General.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/Io/FileManagement.hpp"
#include "Nebulite/Utility/Promise.hpp"
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
        return matchIterator.index;
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
    case DomainRenderingFlags::Alignment::top:
        ImGui::SetNextWindowPos(topPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x, vpSize.y * 0.5f),
            ImGuiCond_Always
        );
        break;
    case DomainRenderingFlags::Alignment::bottom:
        ImGui::SetNextWindowPos(bottomPos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x, vpSize.y * 0.5f),
            ImGuiCond_Always
        );
        break;
    case DomainRenderingFlags::Alignment::left:
        ImGui::SetNextWindowPos(leftPos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x * 0.5f, vpSize.y),
            ImGuiCond_Always
        );
        break;
    case DomainRenderingFlags::Alignment::right:
        ImGui::SetNextWindowPos(rightPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowSize(
            ImVec2(vpSize.x * 0.5f, vpSize.y),
            ImGuiCond_Always
        );
        break;
    case DomainRenderingFlags::Alignment::none:
        break;
    default:
        std::unreachable();
    }
}

void ImguiHelper::domainWindowSetup(DomainRenderingFlags const& flags) {
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
}

void ImguiHelper::createDomainWindowHeader(DomainRenderingFlags const& flags, std::array<FieldData, ViewerLayout::count>& fields, std::string const& windowName, Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture){
    // Header row: Minimized fields, close button (optional)
    ImGui::SameLine();
    renderMinimizeTray(fields);
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

void ImguiHelper::renderDomain(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name, DomainRenderingFlags const& flags) {
    auto const& scope = ctxScope.self;
    std::string const additionalIdentifier = !ctx.self.capture.hasParent() ? "GLOBAL" : "";
    std::string const identifier = name + "_" + std::to_string(ctx.self.getId()) + "_" + additionalIdentifier;
    std::string const windowName = "Nebulite Domain Interface - " + name;
    std::string const windowIdentifier = windowName + "###DomainViewer_" + identifier;

    // Viewer layout
    static std::unordered_map<std::string, ViewerLayout> layouts;
    ViewerLayout& layout = layouts[identifier];

    // Available fields
    std::array fields = {
        FieldData{.title="Console", .state=layout.console, .renderFunction=[&]{renderDomainConsole(ctx, ctxScope, capture, name);}},
        FieldData{.title="JSON", .state=layout.json, .renderFunction=[&]{renderJsonTreeNode(scope, scope.getRootScope());}},
        FieldData{.title="Plot", .state=layout.plot, .renderFunction=[&]{renderPlotViewer(ctx, ctxScope, capture, name);}}
    };
    static_assert(fields.size() == ViewerLayout::count, "Please update the field render logic to reflect the changes in ViewerLayout.");

    // Create window
    domainWindowSetup(flags);
    ImGui::Begin(windowIdentifier.c_str());
    createDomainWindowHeader(flags, fields, windowName, ctx, ctxScope, capture);
    ImGui::Separator();

    // Render visible fields
    int const visibleCount = std::accumulate(fields.begin(), fields.end(), 0, [](int count, FieldData const& field) {
        return count + (field.state == ViewerState::Visible ? 1 : 0);
    });
    if (visibleCount > 0) {
        int columnId = 0;
        ImGui::BeginTable(
            "Viewers",
            visibleCount,
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_Borders
        );

        for (auto& field : fields) {
            if (field.state == ViewerState::Visible) {
                ImGui::TableNextColumn();
                renderViewerTile(columnId, field.title, field.state, field.renderFunction);
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void ImguiHelper::renderMinimizeTray(std::array<FieldData, ViewerLayout::count>& fields) {
    // Only show info if at least one field is minimized
    if (!std::ranges::any_of(fields, [](FieldData const& field) { return field.state == ViewerState::Minimized; })) {
        return;
    }

    // Show minimized fields
    ImGui::TextUnformatted("Minimized:");
    for (auto& field : fields) {
        if (field.state == ViewerState::Minimized) {
            ImGui::SameLine();
            if (ImGui::Button(field.title.c_str())) {
                field.state = ViewerState::Visible;
            }
        }
    }
}

void ImguiHelper::renderViewerTile(int& id, std::string const& title, ViewerState& state, std::function<void()> const& content) {
    // Begin Child
    ImGui::PushID(id++);
    ImGui::BeginChild(
        title.c_str(),
        ImVec2(0, 0),
        true
    );

    // Header
    ImGui::TextUnformatted(title.c_str());
    ImGui::SameLine();
    const float buttonWidth = ImGui::CalcTextSize("Minimize").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float const scrollbarWidth = ImGui::GetScrollMaxY() > 0.0f ? ImGui::GetStyle().ScrollbarSize : 0.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - buttonWidth - scrollbarWidth);
    if (ImGui::Button("Minimize")) {
        state = ViewerState::Minimized;
    }

    // Content
    ImGui::Separator();
    content();

    // Done
    ImGui::EndChild();
    ImGui::PopID();
}

void ImguiHelper::renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKeyView const& root) {
    for (auto const& key : s.listAvailableKeys(root)) {
        std::string const rootPath = root.toString();
        std::string const fullPath = key.view().toString();
        std::string keyPath = fullPath;
        if (rootPath != fullPath) keyPath = fullPath.substr(rootPath.length());
        if (!keyPath.empty() && keyPath.front() == Data::Json::SpecialCharacter::dot) keyPath.erase(0, 1);
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

void ImguiHelper::renderDomainConsole(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name) {
    //------------------------------------------
    // Console output area

    ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
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

    ImGui::EndChild();

    //------------------------------------------
    // Console input area

    ImGui::Separator();

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
            Global::instance().notifyEvent(ctx.self.parseStr(__FUNCTION__ + std::string(" ") + command, ctx, ctxScope));
            command.clear();
            state.historyIndex = 0; // Reset history index after executing a command
        }
        ImGui::SetKeyboardFocusHere(-1);    // focus again
    }
}

namespace {
ImVec4 colorFromIndex(std::size_t i, std::size_t n) {
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
    std::array<char, 256> bufX;
    std::array<char, 256> bufY;
    Interaction::Logic::Expression expressionX;
    Interaction::Logic::Expression expressionY;
    std::vector<std::pair<double, double>> points;
    ImVec4 color;
    Labels labels;
};

struct Plots {
    std::uint16_t idCounter = 0;
    std::list<PlotData> plots;

    std::array<char, 256> bufXMin = {};
    std::array<char, 256> bufXMax = {};
    std::array<char, 256> bufYMin = {};
    std::array<char, 256> bufYMax = {};

    Interaction::Logic::Expression expressionXMin;
    Interaction::Logic::Expression expressionXMax;

    Interaction::Logic::Expression expressionYMin;
    Interaction::Logic::Expression expressionYMax;

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
};

template<std::size_t N = 256>
void imguiLinkedInput(std::array<char, N>& buf, Interaction::Logic::Expression& expr, char const* label) {
    static auto constexpr yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    static auto constexpr red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

    bool const pushedColour = [&] {
        if (!expr.isReturnableAsDouble()) { // TODO: does not work for malformed expressions such as "$("
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
        expr = Interaction::Logic::Expression{buf.data()};
    }
    if (pushedColour) {
        ImGui::PopStyleColor();
    }
}

template<std::size_t N, typename F>
void imguiTable(char const* label, std::array<char const*, N> c, F&& f) {
    auto constexpr columns = c.size();
    ImGui::BeginTable(label, columns, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders);
    ImGui::TableNextRow();
    for (auto const& desc : c) {
        ImGui::TableNextColumn();
        ImGui::Text("%s", desc);
    }
    std::invoke(std::forward<F>(f));
    ImGui::EndTable();
}

} // namespace

void ImguiHelper::renderPlotViewer(Interaction::Context& /*ctx*/, Interaction::ContextScope& ctxScope, Utility::Io::Capture& /*capture*/, std::string const& identifier){
    static std::unordered_map<std::string, Plots> plots;
    auto& plotMetaData = plots[identifier];
    auto& idCounter = plotMetaData.idCounter;
    auto& availablePlots = plotMetaData.plots;

    // Window
    ImGui::BeginChild("PlotViewer", ImVec2(0, 0), true);
    ImGui::Separator();

    // Add Plot
    ImGui::TextUnformatted("Available Plots:");
    ImGui::SameLine();
    static auto constexpr addPlotText = " + Add Plot";
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - ImGui::CalcTextSize(addPlotText).x - ImGui::GetStyle().FramePadding.x * 2.0f);
    if (ImGui::Button(addPlotText)) {
        auto& plot = availablePlots.emplace_back(PlotData{
            .bufX = {""},
            .bufY = {""},
            .expressionX = Interaction::Logic::Expression{"$({global:time.t})"},
            .expressionY = Interaction::Logic::Expression{"$(sin({global:time.t}))"},
            .points = std::vector<std::pair<double, double>>{},
            .color = colorFromIndex(availablePlots.size(), 10),
            .labels = Labels{idCounter++},
        });
        std::strncpy(plot.bufX.data(), plot.expressionX.getFullExpression().c_str(), plot.bufX.size() - 1);
        std::strncpy(plot.bufY.data(), plot.expressionY.getFullExpression().c_str(), plot.bufY.size() - 1);
    }

    // Table of available plots
    imguiTable("AvailablePlotsTable", std::array{"X Expression", "Y Expression", "Colour", "Remove"}, [&] {
        for (auto it = availablePlots.begin(); it != availablePlots.end();) {
            auto& [bufX, bufY, x, y, points, colour, labels] = *it;
            ImGui::TableNextRow();

            // [X input]
            ImGui::TableNextColumn();
            imguiLinkedInput(bufX, x, labels.nameX.data());

            // [Y input]
            ImGui::TableNextColumn();
            imguiLinkedInput(bufY, y, labels.nameY.data());

            // [Colour input]
            ImGui::TableNextColumn();
            ImGui::ColorEdit4(labels.nameColour.c_str(), reinterpret_cast<float*>(&colour), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

            // [Remove button]
            ImGui::TableNextColumn();
            if (ImGui::Button(labels.remove.c_str())) {
                it = availablePlots.erase(it);
            } else {
                it++;
            }
        }
    });

    //------------------------------------------
    // Axis limits
    ImGui::Separator();
    ImGui::BeginTable("LimitsTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders);
    ImGui::TableNextRow();

    // Min/Max for x and y
    ImGui::TableNextColumn();
    imguiLinkedInput(plotMetaData.bufXMin, plotMetaData.expressionXMin, "xMin");
    ImGui::TableNextColumn();
    imguiLinkedInput(plotMetaData.bufXMax, plotMetaData.expressionXMax, "xMax");
    ImGui::TableNextColumn();
    imguiLinkedInput(plotMetaData.bufYMin, plotMetaData.expressionYMin, "yMin");
    ImGui::TableNextColumn();
    imguiLinkedInput(plotMetaData.bufYMax, plotMetaData.expressionYMax, "yMax");
    ImGui::EndTable();

    // Plots ...
    ImGui::Separator();

    if (plotMetaData.validLimits()) {
        double const xMin = plotMetaData.expressionXMin.evalAsDouble(ctxScope, Utility::Promise<&Interaction::Logic::Expression::isReturnableAsDouble>());
        double const xMax = plotMetaData.expressionXMax.evalAsDouble(ctxScope, Utility::Promise<&Interaction::Logic::Expression::isReturnableAsDouble>());
        double const yMin = plotMetaData.expressionYMin.evalAsDouble(ctxScope, Utility::Promise<&Interaction::Logic::Expression::isReturnableAsDouble>());
        double const yMax = plotMetaData.expressionYMax.evalAsDouble(ctxScope, Utility::Promise<&Interaction::Logic::Expression::isReturnableAsDouble>());

        // Render limits for now
        ImGui::Text("Limits: xMin=%.2f, xMax=%.2f, yMin=%.2f, yMax=%.2f", xMin, xMax, yMin, yMax);
    } else {
        ImGui::Text("Invalid plot limits.");
    }


    ImGui::EndChild();
}

} // namespace Nebulite::Graphics
