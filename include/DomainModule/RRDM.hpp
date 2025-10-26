/**
 * @file RRDM.hpp
 * @brief Header file for Renderer DomainModules initialization.
 */

//------------------------------------------
// Basic includes
#include "Constants/DomainSettings.hpp"
#include "Core/Renderer.hpp"

//------------------------------------------
// Module includes 
#if RRDM_ENABLED
    #include "DomainModule/Renderer/RRDM_General.hpp"
    #include "DomainModule/Renderer/RRDM_Console.hpp"
    #include "DomainModule/Renderer/RRDM_RenderObjectDraft.hpp"
#endif

//------------------------------------------
namespace Nebulite::DomainModule {
/**
 * @brief Inserts all DomainModules into the Renderer domain.
 */
void RRDM_init(Nebulite::Core::Renderer* target){
    #if RRDM_ENABLED
        // Initialize DomainModules
        using namespace Nebulite::DomainModule::Renderer;
        target->initModule<General>("Renderer General Functions");
        target->initModule<Console>("Renderer Console Functions");
        target->initModule<RenderObjectDraft>("Renderer RenderObjectDraft Functions");
    #endif
}
} // namespace Nebulite::DomainModule