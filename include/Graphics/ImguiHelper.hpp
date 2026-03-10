#ifndef NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
#define NEBULITE_GRAPHICS_IMGUI_HELPER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JsonScope.hpp"

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
};

} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_IMGUI_HELPER_HPP
