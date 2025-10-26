/**
 * @file RRDM_General.hpp
 * @brief Provides rendering utilities for the Nebulite engine.
 *
 * This file contains a DomainModule for basic rendering-related functioncalls.
 */

#ifndef NEBULITE_RRDM_GENERAL_HPP
#define NEBULITE_RRDM_GENERAL_HPP

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core{
    class Renderer;     // Forward declaration of domain class Renderer
    class RenderObject; // Forward declaration of RenderObject
}

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::General
 * @brief Basic Renderer-Related Functions
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, General){
public:
    /**
     * @brief The Renderer DomainModule does not make use of any Render-Updates yet. This function is empty.
     * 
     * If we ever wish to implement special rendering features, we can do so here.
     * However, the Nebulite::Core::Renderer class takes care of the core rendering functionality.
     * 
     * Why not implement here? Because all DomainModules are called, then we call the Renderer update function.
     * Implementing all renderer updates here breaks this separation, as we would then have to specify the update order
     * of the GlobalSpace DomainModules.
     * 
     * Current implementation is, simplified:
     * 
     * ```cpp
     * while(true){
     *      globalSpace.update();               // Update with all its DomainModules
     *      globalSpace.getRenderer()->tick();  // Update Renderer
     * }
     * ```
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    // Spawn a renderobject
    /**
     * @brief Spawns a new render object.
     * 
     * Memory management ist handled by the Renderer
     * 
     * Implementing `unique_ptr` or `shared_ptr` is a work in progress
     * as its made difficult by the ability of globalspace to select a RenderObject
     * and store its pointer.
     * The Renderer is, besides the selection addition from `selectedObject_get`,
     * a closed system that handles the pointer and lifetime of RenderObjects.
     * Thus, the usage of `unique_ptr` or `shared_ptr` is not needed here, 
     * but perhaps helpful if complexity increases.
     * 
     * We might wish to implement this in the future, but for now
     * we will use a raw pointer and let the Renderer handle the memory management.
     * 
     * Implementing a shared or unique pointer would require a significant rework of:
     * 
     * - Renderer append function
     * 
     * - Environment append function
     * 
     * - RenderObjectContainer append function
     * 
     * - Its batch management
     * 
     * - RenderObject selection mechanism
     * 
     * - RenderObject deletion mechanism in Renderer::update()
     * 
     * As of now, the implementation is fully functional so its a low priority task.
     * 
     * @param argc The argument count
     * @param argv The argument vector: RenderObject as link to json/jsonc file
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error spawn(int argc,  char** argv);
    static std::string const spawn_name;
    static std::string const spawn_desc;

    /**
     * @brief Loads an environment.
     * 
     * @param argc The argument count
     * @param argv The argument vector: Environment as link to json/jsonc file
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error env_load(int argc,  char** argv);
    static std::string const env_load_name;
    static std::string const env_load_desc;

    /**
     * @brief Deload entire environment, leaving an empty renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error env_deload(int argc,  char** argv);
    static std::string const env_deload_name;
    static std::string const env_deload_desc;

    /**
     * @brief Sets resolution of renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: [Width] [Height] [Scale]. 
     * 
     * Defaults to 1    for scale if argument count < 3
     * 
     * Defaults to 1000 for height if argument count < 2
     * 
     * Defaults to 1000 for width if argument count < 1
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error setResolution(int argc,  char** argv);
    static std::string const setResolution_name;
    static std::string const setResolution_desc;

    /**
     * @brief Sets fps of renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: [fps]
     * 
     * Defaults to 60 fps if no argument is provided
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error setFPS(int argc,  char** argv);
    static std::string const setFPS_name;
    static std::string const setFPS_desc;

    /**
     * @brief Toggle fps on/off
     * 
     * @param argc The argument count
     * @param argv The argument vector: [on|off]
     * 
     * Defaults to on if no argument is provided
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error showFPS(int argc,  char** argv);
    static std::string const showFPS_name;
    static std::string const showFPS_desc;

    /**
     * @brief Move cam by a given delta
     * 
     * @param argc The argument count
     * @param argv The argument vector: <dx> <dy>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error cam_move(int argc,  char** argv);
    static std::string const cam_move_name;
    static std::string const cam_move_desc;

    /**
     * @brief Set cam to concrete position
     * 
     * @param argc The argument count
     * @param argv The argument vector: <x> <y> [c]
     * 
     * Where [c] determines if the given position is the camera's center
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error cam_set(int argc,  char** argv);
    static std::string const cam_set_name;
    static std::string const cam_set_desc;

    /**
     * @brief Create a snapshot of the current renderer screen output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <filename>
     * 
     * Defaults to "./Resources/Snapshots/snapshot.png" if no argument is provided
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error snapshot(int argc,  char** argv);
    static std::string const snapshot_name;
    static std::string const snapshot_desc;

    /**
     * @brief Makes a beep noise
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error beep(int argc,  char** argv);
    static std::string const beep_name;
    static std::string const beep_desc;

    /**
     * @brief Inserts a reference to the selected renderobject by ID to the GlobalSpace
     * 
     * If the user ever needs to get an object from the renderer by its ID
     * If an object is found with the given ID, its pointer is stored in GlobalSpace::selectedRenderObject
     * If no object is found, GlobalSpace::selectedRenderObject is set to nullptr
     * 
     * @param argc The argument count
     * @param argv The argument vector: <id>
     * @return Potential errors that occured on command execution
     * 
     * @todo If an object is deleted, the reference in GlobalSpace::selectedRenderObject is not cleared!
     * Fix idea: Make Renderer a domain itself, with this function as a domainmodule part
     * Then, we can have the SelectedRenderObject as a private member of the Renderer domain
     * and manipulate it directly in Renderer::update()
     * This would also make sense, as the Renderer is the owner of the RenderObjects
     * and should thus also be the owner of the selected object reference
     */
    Nebulite::Constants::Error selectedObject_get(int argc,  char** argv);
    static std::string const selectedObject_get_name;
    static std::string const selectedObject_get_desc;

    /**
     * @brief Parses a command on the selected RenderObject
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error selectedObject_Parse(int argc,  char** argv);
    static std::string const selectedObject_Parse_name;
    static std::string const selectedObject_Parse_desc;

    //------------------------------------------
    // Category names
    static std::string const cam_name;
    static std::string const cam_desc;

    static std::string const selectedObject_name;
    static std::string const selectedObject_desc;

    static std::string const env_name;
    static std::string const env_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     * 
     * @todo Move functions for Renderer and Environment to domains themselves,
     * once they are implemented as such.
     * 
     * This will declutter the globalspace, separating its usage from the Renderer and Environment.
     * The only downside currently is that we have to implement a method to lazy-init the SDL Renderer within the Renderer domain itself.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, General){
        bindFunction(&General::spawn,               spawn_name,            &spawn_desc);
        bindFunction(&General::setResolution,       setResolution_name,    &setResolution_desc);
        bindFunction(&General::setFPS,              setFPS_name,           &setFPS_desc);
        bindFunction(&General::showFPS,             showFPS_name,          &showFPS_desc);
        bindFunction(&General::snapshot,            snapshot_name,         &snapshot_desc);
        bindFunction(&General::beep,                beep_name,             &beep_desc);

        bindCategory(cam_name, &cam_desc);
        bindFunction(&General::cam_move,             cam_move_name,         &cam_move_desc);
        bindFunction(&General::cam_set,              cam_set_name,          &cam_set_desc);

        bindCategory(selectedObject_name, &selectedObject_desc);
        bindFunction(&General::selectedObject_get,   selectedObject_get_name,   &selectedObject_get_desc);
        bindFunction(&General::selectedObject_Parse, selectedObject_Parse_name, &selectedObject_Parse_desc);

        bindCategory(env_name, &env_desc);
        bindFunction(&General::env_load,             env_load_name,         &env_load_desc);
        bindFunction(&General::env_deload,           env_deload_name,       &env_deload_desc);
    }

private:
    /**
     * @brief Pointer to the currently selected RenderObject
     */
    Nebulite::Core::RenderObject* selectedRenderObject = nullptr;
};
}   // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_RRDM_GENERAL_HPP