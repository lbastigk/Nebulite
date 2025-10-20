/**
 * @file GSDM_Debug_Domain_Collision_Detection.hpp
 * @brief Contains the Debug Domain Module for testing collision detection functionalities of Domains/DomainModules.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Debug_Domain_Collision_Detection
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Debug_Domain_Collision_Detection) {
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Tests collision detection functionalities of functions.
     * 
     * @param argc The argument count
     * @param argv The argument vector: None.
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error debug_collisionDetect_function(int argc,  char* argv[]);
    static const std::string debug_collisionDetect_function_name;
    static const std::string debug_collisionDetect_function_desc;

    /**
     * @brief Tests collision detection functionalities of categories.
     * 
     * @param argc The argument count
     * @param argv The argument vector: None.
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error debug_collisionDetect_category(int argc,  char* argv[]);
    static const std::string debug_collisionDetect_category_name;
    static const std::string debug_collisionDetect_category_desc;

    /**
     * @brief Tests collision detection functionalities of variables.
     * 
     * @param argc The argument count
     * @param argv The argument vector: None.
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error debug_collisionDetect_variable(int argc,  char* argv[]);
    static const std::string debug_collisionDetect_variable_name;
    static const std::string debug_collisionDetect_variable_desc;

    //------------------------------------------
    // Category names
    static const std::string debug_name;
    static const std::string debug_desc;

    static const std::string collisionDetect_name;
    static const std::string collisionDetect_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug_Domain_Collision_Detection){
        // Bind the categories for the functions
        bindCategory(debug_name, &debug_desc);
        bindCategory(collisionDetect_name, &collisionDetect_desc);

        // Function bindings
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_function, debug_collisionDetect_function_name, &debug_collisionDetect_function_desc);
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_category, debug_collisionDetect_category_name, &debug_collisionDetect_category_desc);
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_variable, debug_collisionDetect_variable_name, &debug_collisionDetect_variable_desc);
    }

private:
};
}   // namespace Nebulite::DomainModule::GlobalSpace