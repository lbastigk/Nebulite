/**
 * @file TXDM.hpp
 * @brief Header file for Texture DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/DomainSettings.hpp"
#include "Core/Texture.hpp"

//------------------------------------------
// Module includes 
#if TXDM_ENABLED
    #include "DomainModule/Texture/TXDM_General.hpp"
    #include "DomainModule/Texture/TXDM_Rotation.hpp"
    #include "DomainModule/Texture/TXDM_Fill.hpp"
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the Texture domain.
 */
void TXDM_init(Nebulite::Core::Texture* target){
    #if TXDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::Texture;
        target->initModule<General>("Texture General Functions");
        target->initModule<Rotation>("Texture Rotation Functions");
        target->initModule<Fill>("Texture Fill Functions");
    #endif
}
}
}