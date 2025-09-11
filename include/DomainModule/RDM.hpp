/**
 * @file RDM.hpp
 * @brief Header file for RenderObject Domain Modules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Module includes 
#if RDM_ENABLED
    #include "DomainModule/RenderObject/RDM_Layout.hpp"
    #include "DomainModule/RenderObject/RDM_Logging.hpp"
    #include "DomainModule/RenderObject/RDM_Parenting.hpp"
    #include "DomainModule/RenderObject/RDM_StateUpdate.hpp"
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
        target->createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Layout>();
        target->createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Logging>();
        target->createDomainModuleOfType<Nebulite::DomainModule::RenderObject::Parenting>();
        target->createDomainModuleOfType<Nebulite::DomainModule::RenderObject::StateUpdate>();
    #endif
}
}
}