/**
 * @file RDM.hpp
 * @brief Header file for RenderObject DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Module includes 
#if RDM_ENABLED
    #include "DomainModule/RenderObject/RDM_Debug.hpp"
    #include "DomainModule/RenderObject/RDM_Logging.hpp"
    #include "DomainModule/RenderObject/RDM_Parenting.hpp"
    #include "DomainModule/RenderObject/RDM_StateUpdate.hpp"
    #include "DomainModule/RenderObject/RDM_Mirror.hpp"
#endif

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the RenderObject domain.
 */
void RDM_init(Nebulite::Core::RenderObject* target){
    #if RDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::RenderObject;
        target->initModule<Debug>("RenderObject Debug Functions");
        target->initModule<Logging>("RenderObject Logging Functions");
        target->initModule<Parenting>("RenderObject Parenting Functions");
        target->initModule<StateUpdate>("RenderObject State Update Functions");
        target->initModule<Mirror>("RenderObject Mirror Functions");
    #endif
}
}
}