/**
 * @file JDM.hpp
 * @brief Header file for JSON Domain Modules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Module includes 
#if JDM_ENABLED
    #include "DomainModule/JSON/JDM_SimpleData.hpp"
    #include "DomainModule/JSON/JDM_ComplexData.hpp"
#endif
//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the JSON domain.
 */
void JDM_init(Nebulite::Utility::JSON* target){
    #if JDM_ENABLED
        // Initialize DomainModules
        target->createDomainModuleOfType<Nebulite::DomainModule::JSON::SimpleData>();
        target->createDomainModuleOfType<Nebulite::DomainModule::JSON::ComplexData>();
    #endif
}
}
}