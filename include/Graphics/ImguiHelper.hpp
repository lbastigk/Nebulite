#ifndef GRAPHICS_IMGUIHELPER_HPP
#define GRAPHICS_IMGUIHELPER_HPP

//------------------------------------------
// Includes

// Standard Library
#include <cstdint>
#include <optional>
#include <string>

// External
#include <imgui.h>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Utility/IO/Capture.hpp"

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

//------------------------------------------
namespace Nebulite::Graphics {

class ImguiHelper {
public:
    struct DomainRenderingFlags {
        bool showCloseButton = true; // Whether to show the close button in the ImGui window when rendering a domain
        std::optional<ImVec2> windowPos = std::nullopt; // Optional position
        std::optional<ImVec2> windowSize = std::nullopt; // Optional size

        enum class Alignment : std::uint8_t {
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
        DomainRenderingFlags const& flags = {.showCloseButton = true, .windowPos = std::nullopt, .windowSize = std::nullopt, .windowAlignment = std::nullopt}
    );

private:
    /**
     * @brief Imgui alignment helper, call before Imgui::Begin().
     * @param alignment The flags for the window alignment.
     */
    static void align(DomainRenderingFlags::Alignment const& alignment);

    /**
     * @brief Renders a JSON tree node in an ImGui window.
     * @param s The JSON scope to render.
     * @param root The root key for the JSON node.
     */
    static void renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKeyView const& root);

    /**
     * @brief Renders the domain console + capture viewer in an ImGui window.
     * @param ctx The context of the caller.
     * @param ctxScope The scope of the caller.
     * @param capture The capture to render.
     * @param name The name of the ImGui window.
     */
    static void renderDomainConsole(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, Utility::IO::Capture& capture, std::string const& name);
};

} // namespace Nebulite::Graphics
#endif // GRAPHICS_IMGUIHELPER_HPP
