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

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/ImguiHelper.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Domain/Common/General.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

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

void ImguiHelper::setCursorPosXForRightAlignedButton(char const* buttonLabel) {
    float const buttonWidth = ImGui::CalcTextSize(buttonLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float const scrollbarWidth = ImGui::GetScrollMaxY() > 0.0f ? ImGui::GetStyle().ScrollbarSize : 0.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - buttonWidth - scrollbarWidth);
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

void ImguiHelper::domainViewerSetup(DomainRenderingFlags const& flags) {
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

void ImguiHelper::renderDomainViewerHeader(DomainRenderingFlags const& flags, std::array<FieldData, ViewerLayout::count>& fields, std::string const& windowName, Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture){
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

void ImguiHelper::renderDomainViewer(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name, DomainRenderingFlags const& flags) {
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
                renderDomainViewerConsole(ctx, ctxScope, capture, name);
            },
        },
        FieldData{
            .title="JSON",
            .state=json,
            .renderFunction=[&] {
                renderJsonTreeNode(scope, scope.getRootScope());
            },
        },
        FieldData{
            .title="Plot",
            .state=plot,
            .renderFunction=[&] {
                renderPlotViewer(ctxScope, identifier);
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

void ImguiHelper::renderViewerField(int& id, std::string const& title, FieldState& state, std::function<void()> const& content) {
    // Begin Child
    ImGui::PushID(id++);
    imguiChild(title.c_str(), [&] {
        // Header
        ImGui::TextUnformatted(title.c_str());
        ImGui::SameLine();
        setCursorPosXForRightAlignedButton("Minimize");
        if (ImGui::Button("Minimize")) {
            state = FieldState::minimized;
        }

        // Content
        ImGui::Separator();
        content();
    });
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

} // namespace Nebulite::Graphics
