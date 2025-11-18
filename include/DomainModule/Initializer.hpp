/**
 * @file Initializer.hpp
 * @brief Header file for DomainModule initializers.
 * @todo Later on, all features for GlobalSpace, JSON, Renderer, Texture, and RenderObject should be initialized in a more modular way.
 *       E.g. with json files describing which modules to load.
 */

#ifndef NEBULITE_DOMAINMODULE_INITIALIZER_HPP
#define NEBULITE_DOMAINMODULE_INITIALIZER_HPP

//------------------------------------------
// Basic includes
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
// Predeclarations of Domains

namespace Nebulite{
    namespace Core{
        class GlobalSpace;
        class RenderObject;
        class Renderer;
        class Texture;
    }
    namespace Utility{
        class JSON;
    }
}

//------------------------------------------
namespace Nebulite::DomainModule {

/**
 * @class Nebulite::DomainModule::Initializer
 * @brief Initializes DomainModules for various Domains.
 * 
 * @todo Later on, we might consider initialization based on configuration inside the Domain,
 *       e.g., only initialize certain modules if enabled inside the Domain's JSON document.
 */
class Initializer{
public:
    /**
     * @brief Initializes DomainModules for the GlobalSpace Domain.
     * @param target Pointer to the GlobalSpace domain to initialize.
     */
    static void initGlobalSpace(Core::GlobalSpace* target);

    /**
     * @brief Initializes DomainModules for the JSON Domain.
     * @param target Pointer to the JSON domain to initialize.
     */
    static void initJSON(Utility::JSON* target);

    
    static void initRenderObject(Core::RenderObject* target);

    static void initRenderer(Core::Renderer* target);

    static void initTexture(Core::Texture* target);
};
} // namespace Nebulite::DomainModule
#endif // NEBULITE_DOMAINMODULE_INITIALIZER_HPP