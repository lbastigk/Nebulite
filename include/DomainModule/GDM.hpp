/**
 * @file GDM.hpp
 * @brief Header file for GlobalSpace DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/GlobalSpace.hpp"

//------------------------------------------
// Module includes 
#if GDM_ENABLED
    #include "DomainModule/GlobalSpace/GDM_Time.hpp"                // Time management functions
    #include "DomainModule/GlobalSpace/GDM_Console.hpp"             // Console functions for text input and display
    #include "DomainModule/GlobalSpace/GDM_General.hpp"             // General functions like eval, exit, wait, etc.
    #include "DomainModule/GlobalSpace/GDM_Renderer.hpp"            // Renderer functions for graphics and display
    #include "DomainModule/GlobalSpace/GDM_Debug.hpp"               // Debugging and logging functions
    #include "DomainModule/GlobalSpace/GDM_Input.hpp"               // Input handling
    #include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.hpp"   // Mock RenderObject for RenderObject functions in global space
#endif
//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the GlobalSpace domain.
 */
void GDM_init(Nebulite::Core::GlobalSpace* target){
    #if GDM_ENABLED
        //------------------------------------------
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::GlobalSpace;
        target->initModule<General>("Global General Functions");
        target->initModule<Renderer>("Global Renderer Functions");
        target->initModule<Debug>("Global Debug Functions");
        target->initModule<Input>("Global Input Functions");
        target->initModule<RenderObjectDraft>("Global RenderObjectDraft Functions");
        target->initModule<Console>("Global Console Functions");

        //------------------------------------------
        // Time module relies on knowing if anything is locking the time
        // So we need to initialize it last
        // Example: Console might want to halt time while open
        //          if we init time first, it will update before console
        //          thus ignoring the console's halt request being send to renderer
        target->initModule<Time>("Global Time Functions");
        
        //------------------------------------------
        // Initialize Variable Bindings
        target->bindVariable(&target->cmdVars.headless, "headless", "Set headless mode (no renderer)");
        target->bindVariable(&target->cmdVars.recover,  "recover",  "Enable recoverable error mode");
    #endif
}
}
}