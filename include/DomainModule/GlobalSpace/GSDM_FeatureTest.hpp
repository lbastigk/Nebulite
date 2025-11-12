/**
 * @file GSDM_FeatureTest.hpp
 *
 * This file contains the DomainModule of the GlobalSpace for testing features.
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
namespace Nebulite::Core{
    class GlobalSpace; // Forward declaration of domain class GlobalSpace
}   // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::FeatureTest
 * @brief DomainModule for testing features in the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, FeatureTest) {
public:
    /**
     * @brief Override of update.
     */
    Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Builds a funcTree with extra arguments and tests it.
     */
    Constants::Error testFuncTree(std::span<std::string const> const& args);
    static std::string const testFuncTree_name;
    static std::string const testFuncTree_desc;

    //------------------------------------------
    // Category names and descriptions
    static std::string const category_feature_test_name;
    static std::string const category_feature_test_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, FeatureTest){
        //------------------------------------------
        // Binding functions to the FuncTree
        (void)bindCategory(category_feature_test_name, &category_feature_test_desc);
        bindFunction(&FeatureTest::testFuncTree,    testFuncTree_name,  &testFuncTree_desc);
    }
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_FEATURE_TEST_HPP