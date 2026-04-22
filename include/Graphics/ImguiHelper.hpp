#ifndef NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
#define NEBULITE_GRAPHICS_IMGUI_HELPER_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
// Includes

// Standard Library
#include <optional>
#include <string>

// External
#include <imgui.h>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Context.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

class ImguiHelper {
public:
    struct DomainRenderingFlags {
        bool showCloseButton = true; // Whether to show the close button in the ImGui window when rendering a domain
        std::optional<ImVec2> windowPos = std::nullopt; // Optional position
        std::optional<ImVec2> windowSize = std::nullopt; // Optional size

        enum class Alignment {
            NONE, // No automatic alignment, use exact position specified in windowPos
            TOP,
            BOTTOM,
            LEFT,
            RIGHT
        };
        std::optional<Alignment> windowAlignment = std::nullopt; // Optional alignment to position the window relative to the specified position
    };

    /**
     * @brief Checks if imgui is initialized and ready for rendering.
     * @return true if imgui is initialized, false otherwise.
     */
    static bool checkImguiInitialized();

    /**
     * @brief Checks if Imgui is ready to render a frame
     * @return true if imgui is initialized and a frame is started, false otherwise.
     */
    static bool checkImguiReadyForRendering();

    /**
     * @brief Renders a JSON scope in an ImGui window.
     * @details Make sure imgui is initialized and a frame is started before calling this function.
     * @param scope The JSON scope to render.
     * @param name The name of the ImGui window.
     */
    static void renderJsonScope(Data::JsonScope const& scope, std::string const& name);

    /**
     * @brief Renders a Domains scope, name and capture in an ImGui window.
     * @details Make sure imgui is initialized and a frame is started before calling this function.
     * @param ctx The context of the caller
     * @param ctxScope the context scope of the caller
     * @param capture The capture to show output from. Likely domain::capture, but passing the global capture is also possible to show all output.
     * @param name The name of the ImGui window.
     * @param flags Optional rendering flags to control the appearance and behavior of the ImGui window.
     */
    static void renderDomain(
        Interaction::Context& ctx,
        Interaction::ContextScope& ctxScope,
        Utility::IO::Capture& capture,
        std::string const& name,
        DomainRenderingFlags const& flags = {.showCloseButton = true, .windowPos = std::nullopt, .windowSize = std::nullopt, .windowAlignment = std::nullopt});

private:
    static void renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKey const& root);
    static void renderDomainConsole(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::IO::Capture& capture, std::string const& name);
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
