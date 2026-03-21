#include "DomainModule/GlobalSpace/FunctionCollision.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Constants::Event FunctionCollision::update() {
    // No periodic update needed for this domain module
    return Constants::Event::Success;
}

//------------------------------------------
// Available Functions

namespace {
bool testFunctionCollision1(bool const& arg) {
    return arg;
}

bool testFunctionCollision2(bool const& arg) {
    (void)arg; // Avoid unused parameter warning
    return true;
}
} // namespace


Constants::Event FunctionCollision::debug_collisionDetect_function(std::span<std::string const> const& args) const {
    bool fail = true;
    if (args.size() >= 2) {
        if (std::string const& mode = args.at(1); mode == "succeed") {
            fail = false;
        } else if (mode == "fail") {
            fail = true;
        } else {
            return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
        }
    }

    Interaction::Execution::FuncTree<bool,bool const&> testTree("Test",true,true, domain.capture);

    if (fail) {
        // This will fail, as the function name is already registered
        try {
            testTree.bindFunction(&testFunctionCollision1, "test", "Test function for collision detection"); // OK
            testTree.bindFunction(&testFunctionCollision2, "test", "Test function for collision detection"); // This should cause a collision error
        } catch (...) {
            // Binding failed as expected -> no error
            return Constants::Event::Success;
        }
        return Constants::StandardCapture::Warning::Functional::bindingCollisionExpected(domain.capture);
    }
    // Try to bind a new function with a unique name
    try {
        testTree.bindFunction(&testFunctionCollision1, "test", "Test function for collision detection"); // OK
        testTree.bindFunction(&testFunctionCollision1, "test", "Test function for collision detection"); // OK
    } catch (...) {
        // This should not happen
        return Constants::StandardCapture::Warning::Functional::bindingCollision(domain.capture);
    }
    return Constants::Event::Success;
}

Constants::Event FunctionCollision::debug_collisionDetect_category(std::span<std::string const> const& args) const {
    bool fail = true;
    if (args.size() >= 2) {
        if (std::string const& mode = args.at(1); mode == "succeed") {
            fail = false;
        } else if (mode == "fail") {
            fail = true;
        } else {
            return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
        }
    }

    Interaction::Execution::FuncTree<bool,bool const&> testTree("Test",true,true, domain.capture);

    if (fail) {
        // This will fail, as the category name is already registered
        try {
            testTree.bindCategory("test-category", "Test category for collision detection"); // OK
            testTree.bindCategory("test-category", "Test category for collision detection"); // Already exists, should cause a collision error
        } catch (...) {
            // Binding failed as expected -> no error
            return Constants::Event::Success;
        }
        return Constants::StandardCapture::Warning::Functional::bindingCollisionExpected(domain.capture);
    }
    // Try to bind a category once
    try {
        testTree.bindCategory("test-category", "Test category for collision detection"); // OK
    } catch (...) {
        // This should not happen
        return Constants::StandardCapture::Warning::Functional::bindingCollision(domain.capture);
    }

    // Just to be safe, we bind a sub-category as well
    try {
        testTree.bindCategory("test-category test-inner-category", "Test category for collision detection"); // OK
    } catch (...) {
        // This should not happen
        return Constants::StandardCapture::Warning::Functional::bindingCollision(domain.capture);
    }
    return Constants::Event::Success;
}

Constants::Event FunctionCollision::debug_collisionDetect_variable(std::span<std::string const> const& args) const {
    bool fail = true;
    if (args.size() >= 2) {
        if (std::string const& mode = args.at(1); mode == "succeed") {
            fail = false;
        } else if (mode == "fail") {
            fail = true;
        } else {
            return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
        }
    }

    Interaction::Execution::FuncTree<bool,bool> testTree("Test",true,true, domain.capture);
    static bool headless = false;
    const static std::string headless_var_desc = "Indicates whether the application is running in headless mode (without GUI).";

    if (fail) {
        // This will fail, as the variable name is already registered in GlobalSpace
        try {
            testTree.bindVariable(&headless, "headless", headless_var_desc); // OK, as this is the first time we bind "headless" in this tree
            testTree.bindVariable(&headless, "headless", headless_var_desc); // This should cause a collision error
        } catch (...) {
            // Binding failed as expected -> no error
            return Constants::Event::Success;
        }
        return Constants::StandardCapture::Warning::Functional::bindingCollisionExpected(domain.capture);
    }
    try {
        testTree.bindVariable(&headless, "headless", headless_var_desc); // OK
    } catch (...) {
        // This should not happen
        return Constants::StandardCapture::Warning::Functional::bindingCollision(domain.capture);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::GlobalSpace
