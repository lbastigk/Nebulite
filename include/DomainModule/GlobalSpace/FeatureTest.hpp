/**
 * @file FeatureTest.hpp
 * @brief Feature testing DomainModule for GlobalSpace.
 */

#ifndef NEBULITE_GSDM_FEATURE_TEST_HPP
#define NEBULITE_GSDM_FEATURE_TEST_HPP

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
 * @class Nebulite::DomainModule::GlobalSpace::FeatureTest
 * @brief DomainModule for testing features in the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, FeatureTest) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Builds a funcTree with extra arguments and tests it.
     */
    Constants::Error testFuncTree(std::span<std::string const> const& args);
    static std::string_view constexpr testFuncTree_name = "feature-test functree";
    static std::string_view constexpr testFuncTree_desc = "Builds a funcTree with extra arguments and tests it";

    //------------------------------------------
    // Category names and descriptions

    static std::string_view constexpr category_feature_test_name = "feature-test";
    static std::string_view constexpr category_feature_test_desc = "Functions for testing features in the GlobalSpace";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, FeatureTest) {
        //------------------------------------------
        // Binding functions to the FuncTree
        (void)bindCategory(category_feature_test_name, category_feature_test_desc);
        bindFunction(&FeatureTest::testFuncTree, testFuncTree_name, testFuncTree_desc);
    }
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_FEATURE_TEST_HPP
