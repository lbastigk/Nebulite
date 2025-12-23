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

namespace Nebulite::Core {
class Environment;
class GlobalSpace;
class RenderObject;
class Renderer;
class Texture;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JSON;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::DomainModule {

/**
 * @class Nebulite::DomainModule::Initializer
 * @brief Initializes DomainModules for Domains.
 * @todo Later on, we might consider initialization based on configuration inside the Domain,
 *       e.g., only initialize certain modules if enabled inside the Domain's JSON document.
 *       -> This would be the core of a ECS-like system, + the rulesets (hardcoded and json-defined)
 *       for RenderObjects to use
 */
class Initializer {
public:
    /**
     * @brief Initializes DomainModules for the Environment Domain.
     * @param target Pointer to the Environment domain to initialize.
     */
    static void initEnvironment(Core::GlobalSpace* target);

    /**
     * @brief Initializes DomainModules for the GlobalSpace Domain.
     * @param target Pointer to the GlobalSpace domain to initialize.
     */
    static void initGlobalSpace(Core::GlobalSpace* target);

    /**
     * @brief Initializes DomainModules for the JSON Domain.
     * @param target Pointer to the JSON domain to initialize.
     */
    static void initJSON(Data::JSON* target);

    /**
     * @brief Initializes DomainModules for the RenderObject Domain.
     * @param target Pointer to the RenderObject domain to initialize.
     */
    static void initRenderObject(Core::RenderObject* target);

    /**
     * @brief Initializes DomainModules for the Renderer Domain.
     * @param target Pointer to the Renderer domain to initialize.
     */
    static void initRenderer(Core::Renderer* target);

    /**
     * @brief Initializes DomainModules for the Texture Domain.
     * @param target Pointer to the Texture domain to initialize.
     */
    static void initTexture(Core::Texture* target);
};
} // namespace Nebulite::DomainModule
#endif // NEBULITE_DOMAINMODULE_INITIALIZER_HPP
