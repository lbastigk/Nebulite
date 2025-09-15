/**
 * @file TDM.hpp
 * @brief Header file for Texture DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/Texture.hpp"

//------------------------------------------
// Module includes 
#if TDM_ENABLED
    
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the Texture domain.
 */
void TDM_init(Nebulite::Core::Texture* target){
    #if TDM_ENABLED
        // Initialize DomainModules
        //using namespace Nebulite::DomainModule::Texture;
        
    #endif
}
}
}