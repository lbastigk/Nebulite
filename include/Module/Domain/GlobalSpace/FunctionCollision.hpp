/**
 * @file FunctionCollision.hpp
 * @brief Contains the Debug Domain Module for testing collision detection functionalities of Domains/DomainModules.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_DEBUG_DOMAIN_COLLISION_DETECTION_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_DEBUG_DOMAIN_COLLISION_DETECTION_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Data/Document/JSON.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::FunctionCollision
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, FunctionCollision) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event debug_collisionDetect_function(std::span<std::string const> const& args) const ;
    static auto constexpr debug_collisionDetect_function_name = "debug collision-detect function";
    static auto constexpr debug_collisionDetect_function_desc = "Tests collision detection of function names\n"
        "\n"
        "Tries to bind a function name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect function [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";

    [[nodiscard]] Constants::Event debug_collisionDetect_category(std::span<std::string const> const& args) const ;
    static auto constexpr debug_collisionDetect_category_name = "debug collision-detect category";
    static auto constexpr debug_collisionDetect_category_desc = "Tests collision detection of category names\n"
        "\n"
        "Tries to bind a category name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect category [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";

    [[nodiscard]] Constants::Event debug_collisionDetect_variable(std::span<std::string const> const& args) const ;
    static auto constexpr debug_collisionDetect_variable_name = "debug collision-detect variable";
    static auto constexpr debug_collisionDetect_variable_desc = "Tests collision detection of variable names\n"
        "\n"
        "Tries to bind a variable name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect variable [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";


    //------------------------------------------
    // Categories

    static auto constexpr debug_name = "debug";
    static auto constexpr debug_desc = "DomainModule for special debugging capabilities within the GlobalSpace.";

    static auto constexpr debug_collisionDetect_name = "debug collision-detect";
    static auto constexpr debug_collisionDetect_desc = "Utilities for testing collision detection functionalities.";

    static auto constexpr debug_categoryTest1_name = "debug categoryLevel1";
    static auto constexpr debug_categoryTest1_desc = "Test category for debugging purposes.";

    static auto constexpr debug_categoryTest2_name = "debug categoryLevel1 categoryLevel2";
    static auto constexpr debug_categoryTest2_desc = "Nested test category for debugging purposes.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, FunctionCollision) {
        //------------------------------------------
        // Test categories for nested category binding
        bindCategory(debug_name, debug_desc);
        bindCategory(debug_collisionDetect_name,  debug_collisionDetect_desc);
        bindCategory(debug_categoryTest1_name, debug_categoryTest1_desc);
        bindCategory(debug_categoryTest2_name, debug_categoryTest2_desc);

        //------------------------------------------
        // Function bindings
        bindFunction(&FunctionCollision::debug_collisionDetect_function, debug_collisionDetect_function_name, debug_collisionDetect_function_desc);
        bindFunction(&FunctionCollision::debug_collisionDetect_category, debug_collisionDetect_category_name, debug_collisionDetect_category_desc);
        bindFunction(&FunctionCollision::debug_collisionDetect_variable, debug_collisionDetect_variable_name, debug_collisionDetect_variable_desc);
    }
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_DEBUG_DOMAIN_COLLISION_DETECTION_HPP
