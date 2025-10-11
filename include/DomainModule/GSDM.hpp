/**
 * @file GSDM.hpp
 * @brief Header file for GlobalSpace DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/GlobalSpace.hpp"

//------------------------------------------
// Module includes 
#if GSDM_ENABLED
    #include "DomainModule/GlobalSpace/GSDM_Time.hpp"                // Time management functions
    #include "DomainModule/GlobalSpace/GSDM_Console.hpp"             // Console functions for text input and display
    #include "DomainModule/GlobalSpace/GSDM_General.hpp"             // General functions like eval, exit, wait, etc.
    #include "DomainModule/GlobalSpace/GSDM_Renderer.hpp"            // Renderer functions for graphics and display
    #include "DomainModule/GlobalSpace/GSDM_Debug.hpp"               // Debugging and logging functions
    #include "DomainModule/GlobalSpace/GSDM_Input.hpp"               // Input handling
    #include "DomainModule/GlobalSpace/GSDM_RenderObjectDraft.hpp"   // Mock RenderObject for RenderObject functions in global space
#endif
//------------------------------------------
namespace Nebulite{
namespace DomainModule{

static const std::string headless_desc = "Set headless mode (no renderer)";
static const std::string recover_desc  = "Enable recoverable error mode";

/**
 * @brief Inserts all DomainModules into the GlobalSpace domain.
 */
void GSDM_init(Nebulite::Core::GlobalSpace* target){
    #if GSDM_ENABLED
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
        target->bindVariable(&target->cmdVars.headless, "headless", &headless_desc);
        target->bindVariable(&target->cmdVars.recover,  "recover",  &recover_desc);
    #endif
}
}
}