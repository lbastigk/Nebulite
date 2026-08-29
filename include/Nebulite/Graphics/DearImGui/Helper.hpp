#ifndef NEBULITE_GRAPHICS_IMGUIHELPER_HPP
#define NEBULITE_GRAPHICS_IMGUIHELPER_HPP

//------------------------------------------
// Includes

// Standard Library
#include <array>
#include <functional>
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
namespace Nebulite::Graphics::DearImGui {

class Helper {
public:
    struct DomainRenderingFlags {
        bool showCloseButton = true; // Whether to show the close button in the ImGui window when rendering a domain
        std::optional<ImVec2> windowPos = std::nullopt; // Optional position
        std::optional<ImVec2> windowSize = std::nullopt; // Optional size
        std::optional<Align::Alignment> windowAlignment = std::nullopt; // Optional alignment to position the window relative to the specified position
    };

    /**
     * @brief Renders a Domains scope, name and capture in an ImGui window.
     * @details Make sure imgui is initialized and a frame is started before calling this function.
     * @param ctx The context of the caller
     * @param ctxScope the context scope of the caller
     * @param capture The capture to show output from. Likely domain::capture, but passing the global capture is also possible to show all output.
     * @param name The name of the ImGui window.
     * @param flags Optional rendering flags to control the appearance and behavior of the ImGui window.
     */
    static void renderDomainViewer(
        Interaction::Context& ctx,
        Interaction::ContextScope& ctxScope,
        Utility::Io::Capture& capture,
        std::string const& name,
        DomainRenderingFlags const& flags = {.showCloseButton = true, .windowPos = std::nullopt, .windowSize = std::nullopt, .windowAlignment = std::nullopt}
    );

private:
    /**
     * @brief Setup ImGui for the next window based on the provided flags
     * @param flags The flags to consider
     */
    static void domainViewerSetup(DomainRenderingFlags const& flags);

    /**
     * @enum FieldState
     * @brief State of each Field
     */
    enum class FieldState : bool {
        visible,
        minimized,
    };

    /**
     * @brief Layout state of the domain viewer
     */
    struct ViewerLayout {
        FieldState console = FieldState::visible;
        FieldState json = FieldState::visible;
        FieldState plot = FieldState::minimized; // Default to minimized
        static auto constexpr count = 3;
    };
    static_assert(sizeof(ViewerLayout) / sizeof(FieldState) == ViewerLayout::count, "Please update the count of ViewerLayout");

    /**
     * @brief We map each Field in ViewerLayout to its title and content to render.
     */
    struct FieldData {
        std::string title;
        FieldState& state;
        std::function<void()> renderFunction;
    };

    /**
     * @brief Renders the header of the domain viewer. With minimize tray and optional close button
     * @param flags The flags to consider
     * @param fields All available fields
     * @param windowName The name of the window
     * @param ctx The context of the window
     * @param ctxScope The context scope of the window
     * @param capture A capture instance to direct logging/warnings/errors to
     */
    static void renderDomainViewerHeader(DomainRenderingFlags const& flags, std::array<FieldData, ViewerLayout::count>& fields, std::string const& windowName, Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture);

    /**
     * @brief Renders a given field of a domain viewer
     * @param id The id (is incremented per call)
     * @param title The title of the tile
     * @param state The State of the tile
     * @param content The content to render
     */
    static void renderViewerField(int& id, std::string const& title, FieldState& state, std::function<void()> const& content);

    /**
     * @brief Renders the domain console + capture viewer in an ImGui window.
     * @param ctx The context of the caller.
     * @param ctxScope The scope of the caller.
     * @param capture The capture to render.
     * @param name The name of the ImGui window.
     */
    static void renderDomainViewerConsole(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::Io::Capture& capture, std::string const& name);

    /**
     * @brief Renders the domain plotting interface
     * @param ctxScope The scope of the caller.
     * @param identifier The identifier for the plot viewer.
     */
    static void renderPlotViewer(Interaction::ContextScope const& ctxScope, std::string const& identifier);
};

} // namespace Nebulite::Graphics::DearImGui
#endif // NEBULITE_GRAPHICS_IMGUIHELPER_HPP
