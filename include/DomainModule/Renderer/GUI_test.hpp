/**
 * @file GUI_test.hpp
 * @brief GUI test module for the NEBULITE engine.
 */

#ifndef NEBULITE_RRDM_GUI_TEST_HPP
#define NEBULITE_RRDM_GUI_TEST_HPP

//------------------------------------------
// Includes

// External
#include "imgui.h"
#include "imgui_impl_sdl3.h"

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::GUI_test
 * @brief DomainModule for testing GUI functionalities.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, GUI_test) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions


    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, GUI_test) {

    }

private:

};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_RRDM_GUI_TEST_HPP
