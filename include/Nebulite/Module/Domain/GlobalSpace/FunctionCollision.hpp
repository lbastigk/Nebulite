#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FUNCTIONCOLLISION_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FUNCTIONCOLLISION_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

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
class FunctionCollision final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event debug_collisionDetect_function(std::span<std::string_view const> const& args) const ;
    static auto constexpr debug_collisionDetect_functionName = "debug collision-detect function";
    static auto constexpr debug_collisionDetect_functionDesc = "Tests collision detection of function names\n"
        "\n"
        "Tries to bind a function name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect function [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";

    [[nodiscard]] Constants::Event debug_collisionDetect_category(std::span<std::string_view const> const& args) const ;
    static auto constexpr debug_collisionDetect_categoryName = "debug collision-detect category";
    static auto constexpr debug_collisionDetect_categoryDesc = "Tests collision detection of category names\n"
        "\n"
        "Tries to bind a category name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect category [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";

    [[nodiscard]] Constants::Event debug_collisionDetect_variable(std::span<std::string_view const> const& args) const ;
    static auto constexpr debug_collisionDetect_variableName = "debug collision-detect variable";
    static auto constexpr debug_collisionDetect_variableDesc = "Tests collision detection of variable names\n"
        "\n"
        "Tries to bind a variable name to globalspace that is already registered, expecting a collision error.\n"
        "\n"
        "Usage: debug collision-detect variable [fail/succeed]\n"
        "\n"
        "Defaults to fail\n";


    //------------------------------------------
    // Categories

    static auto constexpr debugName = "debug";
    static auto constexpr debugDesc = "DomainModule for special debugging capabilities within the GlobalSpace.";

    static auto constexpr debug_collisionDetectName = "debug collision-detect";
    static auto constexpr debug_collisionDetectDesc = "Utilities for testing collision detection functionalities.";

    static auto constexpr debug_categoryTest1Name = "debug categoryLevel1";
    static auto constexpr debug_categoryTest1Desc = "Test category for debugging purposes.";

    static auto constexpr debug_categoryTest2Name = "debug categoryLevel1 categoryLevel2";
    static auto constexpr debug_categoryTest2Desc = "Nested test category for debugging purposes.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit FunctionCollision(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Test categories for nested category binding
        bindCategory(debugName, debugDesc);
        bindCategory(debug_collisionDetectName,  debug_collisionDetectDesc);
        bindCategory(debug_categoryTest1Name, debug_categoryTest1Desc);
        bindCategory(debug_categoryTest2Name, debug_categoryTest2Desc);

        //------------------------------------------
        // Function bindings
        bindFunction(&FunctionCollision::debug_collisionDetect_function, debug_collisionDetect_functionName, debug_collisionDetect_functionDesc);
        bindFunction(&FunctionCollision::debug_collisionDetect_category, debug_collisionDetect_categoryName, debug_collisionDetect_categoryDesc);
        bindFunction(&FunctionCollision::debug_collisionDetect_variable, debug_collisionDetect_variableName, debug_collisionDetect_variableDesc);
    }
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FUNCTIONCOLLISION_HPP
