#include "DomainModule/GlobalSpace/FunctionCollision.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error FunctionCollision::update() {
    // No periodic update needed for this domain module
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Constants::Error FunctionCollision::debug_collisionDetect_function(int argc, char const** argv){
    bool fail = true;
    if (argc >= 2) {
        if (std::string const mode = argv[1]; mode == "succeed") {
            fail = false;
        } else if (mode == "fail") {
            fail = true;
        } else {
            return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    if (fail) {
        // This will fail, as the function name is already registered in GlobalSpace
        BINDFUNCTION(&FunctionCollision::debug_collisionDetect_function, debug_collisionDetect_function_name, debug_collisionDetect_function_desc);
    } else {
        // Try to bind a new function with a unique name
        BINDFUNCTION(&FunctionCollision::debug_collisionDetect_function, "123456", debug_collisionDetect_function_desc);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error FunctionCollision::debug_collisionDetect_category(int argc, char const** argv) const {
    bool fail = true;
    if (argc >= 2) {
        if (std::string const mode = argv[1]; mode == "succeed") {
            fail = false;
        } else if (mode == "fail") {
            fail = true;
        } else {
            return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    if (fail) {
        // This will fail, as the category name is already registered in GlobalSpace
        if (!bindCategory(collisionDetect_name, debug_collisionDetect_category_desc)) {
            // Binding failed as expected
            return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
        }
    } else {
        // Try to bind a new category with a unique name
        if (!bindCategory("123456", debug_collisionDetect_category_desc)) {
            // This should not happen
            return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
        }

        // Just to be safe, we bind a sub-category as well
        if (!bindCategory("123456 789", debug_collisionDetect_category_desc)) {
            // This should not happen
            return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error FunctionCollision::debug_collisionDetect_variable(int argc, char const** argv) const {
    bool fail = true;
    if (argc >= 2) {
        if (std::string const mode = argv[1]; mode == "succeed") {
            fail = false;
        } else if (mode == "fail") {
            fail = true;
        } else {
            return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
        }
    }

    if (fail) {
        // This will fail, as the variable name is already registered in GlobalSpace
        static bool headless = false;
        const static std::string headless_var_desc = "Indicates whether the application is running in headless mode (without GUI).";
        bindVariable(&headless, "headless", headless_var_desc);
    } else {
        // Try to bind a new variable with a unique name
        static bool testVar = false;
        bindVariable(&testVar, "debug_collision_detect_test_variable", debug_collisionDetect_variable_desc);
    }
    return Constants::ErrorTable::NONE();
}
} // namespace Nebulite::DomainModule::GlobalSpace
