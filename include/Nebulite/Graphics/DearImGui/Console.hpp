#ifndef NEBULITE_GRAPHICS_DEARIMGUI_CONSOLE_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_CONSOLE_HPP

//------------------------------------------
// Includes

// External
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::Console {

size_t findFileSeparatorPositionOrFallback(std::string_view str);

void addRootCompletions(std::string_view input, std::vector<std::string>& completions, Interaction::Execution::Domain const& domain);

void addFileCompletions(std::string_view input, std::vector<std::string>& completions);

void addJsonCompletions(std::string_view input, std::vector<std::string>& completions, Data::JsonScope const& scope);

struct ConsoleState {
    std::string command;
    std::string draftCommand;
    std::size_t historyIndex = 0;
    Utility::Io::Capture* capture = nullptr;
    Interaction::Context* ctx = nullptr;
    Interaction::ContextScope* ctxScope = nullptr;
};

// If all completions start with the same prefix, the completions vector is modified:
// input: "ca", completions: ["car-wash", "car-insurance"] -> ["car-"]
bool checkCompletionsForCommonPrefix(std::string_view input, std::vector<std::string>& completions);

void historyScrollingCallback(ImGuiInputTextCallbackData* data, ConsoleState* state);

void completionCallback(ImGuiInputTextCallbackData* data, ConsoleState const* state);

int consoleInputCallback(ImGuiInputTextCallbackData* data);

} // namespace Nebulite::Graphics::DearImGui::Console
#endif // NEBULITE_GRAPHICS_DEARIMGUI_CONSOLE_HPP
