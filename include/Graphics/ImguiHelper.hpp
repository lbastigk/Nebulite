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
     * @brief Renders a JSON scope in an ImGui window.
     * @details Make sure imgui is initialized and a frame is started before calling this function.
     * @param scope The JSON scope to render.
     * @param name The name of the ImGui window.
     */
    static void renderJsonScope(Data::JsonScope const& scope, std::string const& name);

    /**
     * @brief Renders a Domains scope, name and capture in an ImGui window.
     * @details Make sure imgui is initialized and a frame is started before calling this function.
     * @param domain The domain to render.
     * @param scope The JSON scope to render.
     * @param name The name of the ImGui window.
     */
    static void renderDomain(Interaction::Execution::Domain& domain, Data::JsonScope const& scope, std::string const& name);

private:
    static void renderJsonTreeNode(Data::JsonScope const& s, Data::ScopedKey const& root);
    static void renderDomainConsole(Interaction::Execution::Domain& domain, std::string const& name);
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
