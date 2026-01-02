/**
 * @file FunctionCollision.hpp
 * @brief Contains the Debug Domain Module for testing collision detection functionalities of Domains/DomainModules.
 */

#ifndef NEBULITE_GSDM_DEBUG_DOMAIN_COLLISION_DETECTION_HPP
#define NEBULITE_GSDM_DEBUG_DOMAIN_COLLISION_DETECTION_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Data/Document/JSON.hpp"
#include "Interaction/Execution/DomainModule.hpp"


//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::FunctionCollision
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, FunctionCollision) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error debug_collisionDetect_function(int argc, char const* argv[]);
    static std::string_view constexpr debug_collisionDetect_function_name = "debug collision-detect function";
    static std::string_view constexpr debug_collisionDetect_function_desc = "Tests collision detection of function names\n"
        "\n"
        "Tries to bind a function name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect function [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";

    Constants::Error debug_collisionDetect_category(int argc, char const* argv[]);
    static std::string_view constexpr debug_collisionDetect_category_name = "debug collision-detect category";
    static std::string_view constexpr debug_collisionDetect_category_desc = "Tests collision detection of category names\n"
        "\n"
        "Tries to bind a category name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect category [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";

    Constants::Error debug_collisionDetect_variable(int argc, char const* argv[]);
    static std::string_view constexpr debug_collisionDetect_variable_name = "debug collision-detect variable";
    static std::string_view constexpr debug_collisionDetect_variable_desc = "Tests collision detection of variable names\n"
        "\n"
        "Tries to bind a variable name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect variable [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";


    //------------------------------------------
    // Category names

    static std::string_view constexpr debug_name = "debug";
    static std::string_view constexpr debug_desc = "DomainModule for special debugging capabilities within the GlobalSpace.";

    static std::string_view constexpr collisionDetect_name = "debug collision-detect";
    static std::string_view constexpr collisionDetect_desc = "Utilities for testing collision detection functionalities.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, FunctionCollision) {
        // Bind the categories for the functions
        (void)bindCategory(debug_name,  debug_desc);
        (void)bindCategory(collisionDetect_name,  collisionDetect_desc);

        // Function bindings
        BINDFUNCTION(&FunctionCollision::debug_collisionDetect_function, debug_collisionDetect_function_name, debug_collisionDetect_function_desc);
        BINDFUNCTION(&FunctionCollision::debug_collisionDetect_category, debug_collisionDetect_category_name, debug_collisionDetect_category_desc);
        BINDFUNCTION(&FunctionCollision::debug_collisionDetect_variable, debug_collisionDetect_variable_name, debug_collisionDetect_variable_desc);
    }
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_DEBUG_DOMAIN_COLLISION_DETECTION_HPP
