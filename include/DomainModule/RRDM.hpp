/**
 * @file RRDM.hpp
 * @brief Header file for Renderer DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/Renderer.hpp"

//------------------------------------------
// Module includes 
#if RRDM_ENABLED
    //#include "DomainModule/Renderer/RRDM_Debug.hpp"
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the Renderer domain.
 */
void RRDM_init(Nebulite::Core::Renderer* target){
    #if RRDM_ENABLED
        // Initialize DomainModules
        //using namespace Nebulite::DomainModule::Renderer;
        //target->initModule<Debug>("Renderer Debug Functions");
    #endif
}
}
}