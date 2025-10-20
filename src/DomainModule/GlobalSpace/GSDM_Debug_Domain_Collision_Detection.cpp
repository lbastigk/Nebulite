#include "DomainModule/GlobalSpace/GSDM_Debug_Domain_Collision_Detection.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Nebulite::Constants::Error Debug_Domain_Collision_Detection::update() {
    // No periodic update needed for this domain module
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Category names
const std::string Debug_Domain_Collision_Detection::debug_name = "debug";
const std::string Debug_Domain_Collision_Detection::debug_desc = "DomainModule for special debugging capabilities within the GlobalSpace.";

const std::string Debug_Domain_Collision_Detection::collisionDetect_name = "debug collision-detect";
const std::string Debug_Domain_Collision_Detection::collisionDetect_desc = "Utilities for testing collision detection functionalities.";

//------------------------------------------
// Available Functions

Nebulite::Constants::Error Debug_Domain_Collision_Detection::debug_collisionDetect_function(int argc,  char* argv[]) {
    bool fail = true;
    if(argc >= 2){
        std::string mode = argv[1];
        if(mode == "succeed"){
            fail = false;
        }
        else if(mode == "fail"){
            fail = true;
        }
        else{
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    if(fail){
        // This will fail, as the function name is already registered in GlobalSpace
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_function, debug_collisionDetect_function_name, &debug_collisionDetect_function_desc);
    }
    else{
        // Try to bind a new function with a unique name
        bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_function, "123456", &debug_collisionDetect_function_desc);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_function_name = "debug collision-detect function";
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_function_desc = R"(Tests collision detection of functions names

Tries to bind a function name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect function [fail/succeed]

Defaults to fail
)";

Nebulite::Constants::Error Debug_Domain_Collision_Detection::debug_collisionDetect_category(int argc,  char* argv[]) {
    bool fail = true;
    if(argc >= 2){
        std::string mode = argv[1];
        if(mode == "succeed"){
            fail = false;
        }
        else if(mode == "fail"){
            fail = true;
        }
        else{
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    if(fail){
        // This will fail, as the category name is already registered in GlobalSpace
        bindCategory(collisionDetect_name, &debug_collisionDetect_category_desc);
    }
    else{
        // Try to bind a new category with a unique name
        bindCategory("123456", &debug_collisionDetect_category_desc);

        // Just to be safe, we bind a sub-category as well
        bindCategory("123456 789", &debug_collisionDetect_category_desc);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_category_name = "debug collision-detect category";
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_category_desc = R"(Tests collision detection of categories

Tries to bind a category that is already registered in globalspace, expecting a collision error.

Usage: debug collision-detect category [fail/succeed]

Defaults to fail
)";

Nebulite::Constants::Error Debug_Domain_Collision_Detection::debug_collisionDetect_variable(int argc,  char* argv[]) {
    bool fail = true;
    if(argc >= 2){
        std::string mode = argv[1];
        if(mode == "succeed"){
            fail = false;
        }
        else if(mode == "fail"){
            fail = true;
        }
        else{
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    if(fail){
        // This will fail, as the variable name is already registered in GlobalSpace
        static bool headless = false;
        const static std::string headless_var_desc = "Indicates whether the application is running in headless mode (without GUI).";
        bindVariable(&headless, "headless", &headless_var_desc);
    }
    else{
        // Try to bind a new variable with a unique name
        static bool testVar = false;
        bindVariable(&testVar, "debug_collision_detect_test_variable", &debug_collisionDetect_variable_desc);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_variable_name = "debug collision-detect variable";
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_variable_desc = R"(Tests collision detection of variable names

Tries to bind a variable name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect variable [fail/succeed]

Defaults to fail
)";

}   // namespace Nebulite::DomainModule::GlobalSpace