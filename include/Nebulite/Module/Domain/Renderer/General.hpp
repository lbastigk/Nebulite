#ifndef NEBULITE_MODULE_DOMAIN_RENDERER_GENERAL_HPP
#define NEBULITE_MODULE_DOMAIN_RENDERER_GENERAL_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::General
 * @brief Basic Renderer-Related Functions
 */
class General final : public Base::DomainModule<Core::Renderer> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // Spawn a renderobject
    /**
     * @brief Spawns a new render object.
     * @details Memory management ist handled by the Renderer
     *          Implementing `unique_ptr` or `shared_ptr` is a work in progress
     *          as its made difficult by the ability of globalspace to select a RenderObject
     *          and store its pointer.
     *          The Renderer is, besides the selection addition from `selectedObjectGet`,
     *          a closed system that handles the pointer and lifetime of RenderObjects.
     *          Thus, the usage of `unique_ptr` or `shared_ptr` is not needed here,
     *          but perhaps helpful if complexity increases.
     *          We might wish to implement this in the future, but for now
     *          we will use a raw pointer and let the Renderer handle the memory management.
     *          Implementing a shared or unique pointer would require a significant rework of:
     *          - Renderer append function
     *          - Environment append function
     *          - RenderObjectContainer append function
     *          - Its batch management
     *          - RenderObject selection mechanism
     *          - RenderObject deletion mechanism in Renderer::update()
     *          As of now, the implementation is fully functional so it's a low priority task.
     */
    [[nodiscard]] Constants::Event spawn(std::span<std::string_view const> const& args) const ;
    static auto constexpr spawnName = "spawn";
    static auto constexpr spawnDesc = "Spawn a RenderObject from a json/jsonc file.\n"
        "\n"
        "Usage: spawn <path/to/file.jsonc>\n\n"
        "Supports lookups in standard resource directories:\n"
        "- ./Resources/RenderObjects/\n"
        "- ./Resources/Renderobjects/\n\n"
        "Example: 'spawn Planets/sun.jsonc|set text.str This is a sun'\n"
        "Looks for object 'sun.jsonc' in the standard directories\n"
        "- './Planets/sun.jsonc'\n"
        "- './Resources/RenderObjects/Planets/sun.jsonc'\n"
        "- './Resources/Renderobjects/Planets/sun.jsonc'\n"
        "and spawns the first found object.\n";

    [[nodiscard]] Constants::Event envLoad(std::span<std::string_view const> const& args) const ;
    static auto constexpr envLoadName = "env load";
    static auto constexpr envLoadDesc = "Load an environment/level from a json/jsonc file.\n"
        "\n"
        "Usage: env load <path/to/file.jsonc>\n\n"
        "If no argument is provided, an empty environment is loaded.\n";

    [[nodiscard]] Constants::Event envDeload() const ;
    static auto constexpr envDeloadName = "env deload";
    static auto constexpr envDeloadDesc = "Deload entire environment, leaving an empty renderer.\n"
        "\n"
        "Usage: env deload\n";

    [[nodiscard]] Constants::Event setResolution(int argc, char const** argv) const ;
    static auto constexpr setResolutionName = "set-res";
    static auto constexpr setResolutionDesc = "Set resolution of renderer.\n"
        "\n"
        "Usage: set-res [Width] [Height] [Scale]\n\n"
        "Defaults to 1000  for width if argument count < 1\n"
        "Defaults to 1000  for height if argument count < 2\n"
        "Defaults to 1     for scale if argument count < 3\n";

    [[nodiscard]] Constants::Event setFps(int argc, char const** argv) const ;
    static auto constexpr setFpsName = "set-fps";
    static auto constexpr setFpsDesc = "Set FPS of renderer.\n"
        "\n"
        "Usage: set-fps [fps]\n\n"
        "Defaults to 60 fps if no argument is provided\n";

    [[nodiscard]] Constants::Event showFps(int argc, char const** argv) const ;
    static auto constexpr showFpsName = "show-fps";
    static auto constexpr showFpsDesc = "Show FPS of renderer.\n"
        "\n"
        "Usage: show-fps [on|off]\n\n"
        "Defaults to on if no argument is provided\n";

    [[nodiscard]] Constants::Event camMove(int argc, char const** argv) const ;
    static auto constexpr camMoveName = "cam move";
    static auto constexpr camMoveDesc = "Move camera by a given delta.\n"
        "\n"
        "Usage: cam move <dx> <dy>\n\n"
        "<dx> : Delta x to move camera by\n"
        "<dy> : Delta y to move camera by\n";

    [[nodiscard]] Constants::Event camSet(int argc, char const** argv) const ;
    static auto constexpr camSetName = "cam set";
    static auto constexpr camSetDesc = "Set camera to concrete position.\n"
        "\n"
        "Usage: cam set <x> <y> [c]\n\n"
        "<x> : X position to set camera to\n"
        "<y> : Y position to set camera to\n"
        "[c] : Optional. If provided, sets the camera's center to the given position.\n";

    [[nodiscard]] Constants::Event snapshot(int argc, char const** argv) const ;
    static auto constexpr snapshotName = "snapshot";
    static auto constexpr snapshotDesc = "Create a snapshot of the current renderer state.\n"
        "\n"
        "Usage: snapshot [filename]\n\n"
        "Defaults to \"./Resources/Snapshots/snapshot.png\" if no argument is provided.\n"
        "Snapshots are create asynchronously; a snapshot callback function is added to the renderer that is called after the next render pass.\n";

    [[nodiscard]] Constants::Event dumpView() const;
    static auto constexpr dumpViewName = "dump-view";
    static auto constexpr dumpViewDesc = "Dump the current view of the renderer to the console, as JSON.\n"
        "The dump is not synchronous with the call, but is executed after the next render pass.\n"
        "\n"
        "Usage: dump-view\n";

    //------------------------------------------
    // Categories

    static auto constexpr camName = "cam";
    static auto constexpr camDesc = "Renderer Camera Functions";

    static auto constexpr envName = "env";
    static auto constexpr envDesc = "Environment management functions";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     * @todo Add domainModules for camera and renderobject-selection and move respective functions in there.
     */
    explicit General(ConstructorParams const& params);
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // NEBULITE_MODULE_DOMAIN_RENDERER_GENERAL_HPP
