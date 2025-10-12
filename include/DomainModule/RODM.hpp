/**
 * @file RODM.hpp
 * @brief Header file for RenderObject DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Module includes 
#if RODM_ENABLED
    #include "DomainModule/RenderObject/RODM_Debug.hpp"
    #include "DomainModule/RenderObject/RODM_Logging.hpp"
    #include "DomainModule/RenderObject/RODM_StateUpdate.hpp"
    #include "DomainModule/RenderObject/RODM_Mirror.hpp"
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the RenderObject domain.
 */
void RODM_init(Nebulite::Core::RenderObject* target){
    #if RODM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::RenderObject;
        target->initModule<Debug>("RenderObject Debug Functions");
        target->initModule<Logging>("RenderObject Logging Functions");
        target->initModule<StateUpdate>("RenderObject State Update Functions");
        target->initModule<Mirror>("RenderObject Mirror Functions");
    #endif
}
}
}