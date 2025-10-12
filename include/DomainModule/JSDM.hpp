/**
 * @file JSDM.hpp
 * @brief Header file for JSON DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/CompileSettings.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Module includes 
#if JSDM_ENABLED
    #include "DomainModule/JSON/JSDM_SimpleData.hpp"
    #include "DomainModule/JSON/JSDM_ComplexData.hpp"
    #include "DomainModule/JSON/JSDM_Debug.hpp"
#endif
//------------------------------------------
namespace Nebulite{
namespace DomainModule{
/**
 * @brief Inserts all DomainModules into the JSON domain.
 */
void JSDM_init(Nebulite::Utility::JSON* target){
    #if JSDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::JSON;
        target->initModule<SimpleData>("JSON Simple Data Functions");
        target->initModule<ComplexData>("JSON Complex Data Functions");
        target->initModule<Debug>("JSON Debug Functions");
    #endif
}
}
}