//------------------------------------------
// Includes

// External
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Graphics/ImguiHelper.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

void ImguiHelper::renderJsonScope(Data::JsonScope const& scope, std::string const& name) {
    ImGui::Begin(name.c_str());
    renderJsonTreeNode(scope, scope.getRootScope().toScopedKey());
    ImGui::End();
}

// TODO: Provide a rolling id to domain class for this to work properly. For now we just use the provided name
void ImguiHelper::renderDomain(Interaction::Execution::Domain& domain, Data::JsonScope const& scope, std::string const& name) {
    ImGui::Begin(name.c_str());
    ImGui::Columns(2, nullptr, true);

    ImGui::BeginChild("DomainConsole", ImVec2(0, 0), true);
    renderDomainConsole(domain, name);
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

// TODO: Text input is garbage
void ImguiHelper::renderDomainConsole(Interaction::Execution::Domain& domain, std::string const& name) {
    // Console output area
    ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

    for (const auto& [content, lineType] : domain.capture.getOutputList()){
        switch (lineType) {
            case Utility::OutputLine::Type::COUT:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // white
                break;
            case Utility::OutputLine::Type::CERR:
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // red
                break;
            default:
                std::unreachable();
        }
        ImGui::TextUnformatted(content.c_str());
        ImGui::PopStyleColor();   // <-- required
    }

    // Auto-scroll
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    // Command input
    ImGui::Separator();

    static std::unordered_map<std::string, std::string> commands;
    std::string& command = commands[name];
    if (ImGui::InputText("##ConsoleInput", &command, ImGuiInputTextFlags_EnterReturnsTrue)){
        if (!command.empty()){
            if (auto const err = domain.parseStr(__FUNCTION__ + std::string(" ") + command); err.isError()) {
                domain.capture.error.println(err.getDescription());
            }
        }
        ImGui::SetKeyboardFocusHere(-1);    // focus again
    }
}

} // namespace Nebulite::Graphics
