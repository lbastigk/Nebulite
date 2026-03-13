#ifndef NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
#define NEBULITE_GRAPHICS_IMGUI_HELPER_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
    class JsonScope;
}

namespace Nebulite::Interaction::Execution {
    class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
// Includes

// Standard Library
#include <string>

//------------------------------------------
namespace Nebulite::Graphics {

class ImguiHelper {
public:
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
     * @param domain The domain to render and parse commands into
     * @param capture The capture to show output from. Likely domain::capture, but passing the global capture is also possible to show all output.
     * @param scope The JSON scope to render. Likely from the domain, but passing the global scope is also possible to show all data.
     * @param name The name of the ImGui window.
     */
    static void renderDomain(Interaction::Execution::Domain& domain, Utility::Capture& capture, Data::JsonScope const& scope, std::string const& name);

private:
    static void renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKey const& root);
    static void renderDomainConsole(Interaction::Execution::Domain& domain, Utility::Capture& capture, std::string const& name);
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
