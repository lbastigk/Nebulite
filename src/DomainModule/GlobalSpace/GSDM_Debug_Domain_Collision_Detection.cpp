#include "DomainModule/GlobalSpace/GSDM_Debug_Domain_Collision_Detection.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Nebulite::Constants::Error Debug_Domain_Collision_Detection::update() {
    // No periodic update needed for this domain module
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Subtree names
const std::string Debug_Domain_Collision_Detection::debug_name = "debug";
const std::string Debug_Domain_Collision_Detection::debug_desc = "DomainModule for special debugging capabilities within the GlobalSpace.";

const std::string Debug_Domain_Collision_Detection::collisionDetect_name = "debug collision-detect";
const std::string Debug_Domain_Collision_Detection::collisionDetect_desc = "Utilities for testing collision detection functionalities.";

//------------------------------------------
// Available Functions

Nebulite::Constants::Error Debug_Domain_Collision_Detection::debug_collisionDetect_function([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // This will fail, as the function name is already registered in GlobalSpace
    bindFunction(&Debug_Domain_Collision_Detection::debug_collisionDetect_function, debug_collisionDetect_function_name, &debug_collisionDetect_function_desc);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_function_name = "debug collision-detect function";
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_function_desc = R"(Tests collision detection of functions names

Tries to bind a function name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect function
)";

Nebulite::Constants::Error Debug_Domain_Collision_Detection::debug_collisionDetect_subtree([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // This will fail, as the subtree name is already registered in GlobalSpace
    bindSubtree(debug_collisionDetect_subtree_name, &debug_collisionDetect_subtree_desc);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_subtree_name = "debug collision-detect subtree";
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_subtree_desc = R"(Tests collision detection of subtrees

Tries to bind a subtree that is already registered in globalspace, expecting a collision error.

Usage: debug collision-detect subtree
)";

Nebulite::Constants::Error Debug_Domain_Collision_Detection::debug_collisionDetect_variable([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // This will fail, as the variable name is already registered in GlobalSpace
    static bool headless = false;
    const static std::string headless_var_desc = "Indicates whether the application is running in headless mode (without GUI).";
    bindVariable(&headless, "headless", &headless_var_desc);
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_variable_name = "debug collision-detect variable";
const std::string Debug_Domain_Collision_Detection::debug_collisionDetect_variable_desc = R"(Tests collision detection of variable names

Tries to bind a variable name to globalspace that is already registered, expecting a collision error.

Usage: debug collision-detect variable
)";

}   // namespace Nebulite::DomainModule::GlobalSpace