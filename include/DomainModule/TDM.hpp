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
    #include "DomainModule/Texture/TDM_General.hpp"
    #include "DomainModule/Texture/TDM_Rotation.hpp"
    #include "DomainModule/Texture/TDM_Fill.hpp"
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
        using namespace Nebulite::DomainModule::Texture;
        target->initModule<General>("Texture General Functions");
        target->initModule<Rotation>("Texture Rotation Functions");
        target->initModule<Fill>("Texture Fill Functions");
    #endif
}
}
}