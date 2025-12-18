/**
 * @file Debug.hpp
 * @brief This file contains the DomainModule of the GlobalSpace to provide Debug capabilities.
 */

#ifndef NEBULITE_RRDM_DEBUG_HPP
#define NEBULITE_RRDM_DEBUG_HPP

//------------------------------------------
// Includes

// External
#include <SDL.h>
#include <SDL_ttf.h>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Data/JSON.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TextInput.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core


//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Debug
 * @brief DomainModule for Debug capabilities within the Renderer.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error windowStatus(std::span<std::string const> const& args);
    static std::string const windowStatus_name;
    static std::string const windowStatus_desc;

    //------------------------------------------
    // Category strings

    static std::string const debug_name;
    static std::string const debug_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Debug) {
        bindCategory(debug_name, &debug_desc);
        bindFunction(&Debug::windowStatus, windowStatus_name, &windowStatus_desc);
    }

private:

};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_RRDM_DEBUG_HPP