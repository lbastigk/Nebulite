#include "DomainModule/GlobalSpace/FunctionCollision.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error FunctionCollision::update() {
    // No periodic update needed for this domain module
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Constants::Error FunctionCollision::debug_collisionDetect_function(int const argc, char const** argv){
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
        try {
            BIND_FUNCTION(&FunctionCollision::debug_collisionDetect_function, debug_collisionDetect_function_name, debug_collisionDetect_function_desc);
        } catch (...) {
            // Binding failed as expected -> no error
            return Constants::ErrorTable::NONE();
        }
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION_EXPECTED();
    }
    // Try to bind a new function with a unique name
    try {
        BIND_FUNCTION(&FunctionCollision::debug_collisionDetect_function, "unique_debug_collision_function", debug_collisionDetect_function_desc);
    } catch (...) {
        // This should not happen
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error FunctionCollision::debug_collisionDetect_category(int const argc, char const** argv) const {
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
        try {
            bindCategory(std::string(debug_collisionDetect_name), debug_collisionDetect_category_desc);
        } catch (...) {
            // Binding failed as expected -> no error
            return Constants::ErrorTable::NONE();
        }
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION_EXPECTED();
    }
    // Try to bind a new category with a unique name
    try {
        bindCategory("123456", debug_collisionDetect_category_desc);
    } catch (...) {
        // This should not happen
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
    }

    // Just to be safe, we bind a sub-category as well
    try {
        bindCategory("123456 subcategory", debug_collisionDetect_category_desc);
    } catch (...) {
        // This should not happen
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error FunctionCollision::debug_collisionDetect_variable(int const argc, char const** argv) const {
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
        try {
            bindVariable(&headless, "headless", headless_var_desc);
        } catch (...) {
            // Binding failed as expected -> no error
            return Constants::ErrorTable::NONE();
        }
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION_EXPECTED();
    }
    // Try to bind a new variable with a unique name
    static bool testVar = false;
    try {
        bindVariable(&testVar, "unique_debug_collision_variable", debug_collisionDetect_variable_desc);
    } catch (...) {
        // This should not happen
        return Constants::ErrorTable::FUNCTIONAL::BINDING_COLLISION();
    }
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::GlobalSpace
