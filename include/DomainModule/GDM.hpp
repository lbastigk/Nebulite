/**
 * @file GDM.hpp
 * @brief Header file for GlobalSpace Domain Modules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/GlobalSpace.hpp"

//------------------------------------------
// Module includes 
#if GDM_ENABLED
    #include "DomainModule/GlobalSpace/GDM_General.hpp"             // General functions like eval, exit, wait, etc.
    #include "DomainModule/GlobalSpace/GDM_Renderer.hpp"            // Renderer functions for graphics and display
    #include "DomainModule/GlobalSpace/GDM_Debug.hpp"               // Debugging and logging functions
    #include "DomainModule/GlobalSpace/GDM_GUI.hpp"                 // GUI functions for DearImgui integration
    #include "DomainModule/GlobalSpace/GDM_Input.hpp"               // Input handling
    #include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.hpp"   // Mock RenderObject for RenderObject functions in global space
#endif
//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the JSON domain.
 */
void GDM_init(Nebulite::Core::GlobalSpace* target){
    #if GDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::GlobalSpace;
        target->initModule<General>("Global General Functions");
        target->initModule<Renderer>("Global Renderer Functions");
        target->initModule<Debug>("Global Debug Functions");
        target->initModule<GUI>("Global GUI Functions");
        target->initModule<Input>("Global Input Functions");
        target->initModule<RenderObjectDraft>("Global RenderObjectDraft Functions");
        
        // Initialize Variable Bindings
        target->bindVariable(&target->cmdVars.headless, "headless", "Set headless mode (no renderer)");
        target->bindVariable(&target->cmdVars.recover,  "recover",  "Enable recoverable error mode");
    #endif
}
}
}