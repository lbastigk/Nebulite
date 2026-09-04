#ifndef NEBULITE_GRAPHICS_DEARIMGUI_DOMAINVIEWER_HPP
#define NEBULITE_GRAPHICS_DEARIMGUI_DOMAINVIEWER_HPP

//------------------------------------------
// Includes

// Standard Library
#include <optional>
#include <string>

// External
#include <imgui.h>

// Nebulite
#include "Nebulite/Graphics/DearImGui/Align.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
class ScopedKeyView;
} // namespace Nebulite::Data

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Utility::Io {
class Capture;
} // namespace Nebulite::Utility::Io

//------------------------------------------
namespace Nebulite::Graphics::DearImGui::DomainViewer {
/**
 * @struct RenderingFlags
 * @brief Settings to use for DomainViewer rendering.
 */
struct RenderingFlags {
    bool showCloseButton = true; // Whether to show the close button in the ImGui window when rendering a domain
    std::optional<ImVec2> windowPos = std::nullopt; // Optional position
    std::optional<ImVec2> windowSize = std::nullopt; // Optional size
    std::optional<Align::Alignment> windowAlignment = std::nullopt; // Optional alignment to position the window relative to the specified position
};

/**
 * @brief Renders the domain viewer in an ImGui window.
 * @details Make sure imgui is initialized and a frame is started before calling this function.
 * @param ctx The context of the caller
 * @param ctxScope the context scope of the caller
 * @param capture The capture to show output from. Likely domain::capture, but passing the global capture is also possible to show all output.
 * @param name The name of the ImGui window.
 * @param flags Optional rendering flags to control the appearance and behavior of the ImGui window.
 */
void render(
    Interaction::Context& ctx,
    Interaction::ContextScope& ctxScope,
    Utility::Io::Capture& capture,
    std::string const& name,
    RenderingFlags const& flags = {}
);

/**
 * @brief Renders the domain plotting field.
 * @param ctxScope The scope of the caller.
 * @param identifier The identifier for the plot viewer.
 */
void renderPlotField(Interaction::ContextScope const& ctxScope, std::string const& identifier);

/**
 * @brief Renders the domain console + capture viewer in an ImGui window.
 * @param ctx The context of the caller.
 * @param ctxScope The scope of the caller.
 * @param capture The capture to render.
 * @param name The name of the ImGui window.
 */
void renderConsoleField(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name);

/**
 * @brief Renders the domain JSON viewer in an ImGui window.
 * @param scope The JSON scope to render.
 * @param root The root key of the JSON scope.
 */
void renderJsonField(Data::JsonScope const& scope, Data::ScopedKeyView const& root);
} // namespace Nebulite::Graphics::DearImGui::DomainViewer
#endif // NEBULITE_GRAPHICS_DEARIMGUI_DOMAINVIEWER_HPP
