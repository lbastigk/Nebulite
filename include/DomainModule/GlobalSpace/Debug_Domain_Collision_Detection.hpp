/**
 * @file Debug_Domain_Collision_Detection.hpp
 * @brief Contains the Debug Domain Module for testing collision detection functionalities of Domains/DomainModules.
 */

#ifndef NEBULITE_GSDM_DEBUG_DOMAIN_COLLISION_DETECTION_HPP
#define NEBULITE_GSDM_DEBUG_DOMAIN_COLLISION_DETECTION_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Debug_Domain_Collision_Detection
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Debug_Domain_Collision_Detection) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Tests collision detection functionalities of functions.
     * 
     * @param argc The argument count
     * @param argv The argument vector: None.
     * @return Potential errors that occurred on command execution
     */
    Constants::Error debug_collisionDetect_function(int argc, char const* argv[]);
    static std::string const debug_collisionDetect_function_name;
    static std::string const debug_collisionDetect_function_desc;

    /**
     * @brief Tests collision detection functionalities of categories.
     * 
     * @param argc The argument count
     * @param argv The argument vector: None.
     * @return Potential errors that occurred on command execution
     */
    Constants::Error debug_collisionDetect_category(int argc, char const* argv[]);
    static std::string const debug_collisionDetect_category_name;
    static std::string const debug_collisionDetect_category_desc;

    /**
     * @brief Tests collision detection functionalities of variables.
     * 
     * @param argc The argument count
     * @param argv The argument vector: None.
     * @return Potential errors that occurred on command execution
     */
    Constants::Error debug_collisionDetect_variable(int argc, char const* argv[]);
    static std::string const debug_collisionDetect_variable_name;
    static std::string const debug_collisionDetect_variable_desc;

    //------------------------------------------
    // Category names
    static std::string const debug_name;
    static std::string const debug_desc;

    static std::string const collisionDetect_name;
    static std::string const collisionDetect_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug_Domain_Collision_Detection) {
        // Bind the categories for the functions
        (void)bindCategory(debug_name, &debug_desc);
        (void)bindCategory(collisionDetect_name, &collisionDetect_desc);

        // Function bindings
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_function, debug_collisionDetect_function_name, &debug_collisionDetect_function_desc);
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_category, debug_collisionDetect_category_name, &debug_collisionDetect_category_desc);
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_variable, debug_collisionDetect_variable_name, &debug_collisionDetect_variable_desc);
    }
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_DEBUG_DOMAIN_COLLISION_DETECTION_HPP