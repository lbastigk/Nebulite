/**
 * @file FeatureTest.hpp
 * @brief Feature testing DomainModule for GlobalSpace.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_FEATURE_TEST_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_FEATURE_TEST_HPP

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

    static Constants::Error testFuncTree(std::span<std::string const> const& args);
    static auto constexpr testFuncTree_name = "feature-test functree";
    static auto constexpr testFuncTree_desc = "Builds a funcTree with extra arguments and tests it\n"
        "Usage: feature-test functree\n";

    Constants::Error selfOtherGlobalEvaluation() const ;
    static auto constexpr selfOtherGlobalEvaluation_name = "evaluation";
    static auto constexpr selfOtherGlobalEvaluation_desc = "Tests evaluation of self and other globals in one expression\n"
        "Usage: evaluation\n";


    //------------------------------------------
    // Category names and descriptions

    static auto constexpr category_feature_test_name = "feature-test";
    static auto constexpr category_feature_test_desc = "Functions for testing features in the GlobalSpace\n"
        "Usage: feature-test <function>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, FeatureTest) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindCategory(category_feature_test_name, category_feature_test_desc);
        BIND_FUNCTION(&FeatureTest::testFuncTree, testFuncTree_name, testFuncTree_desc);
        BIND_FUNCTION(&FeatureTest::selfOtherGlobalEvaluation, selfOtherGlobalEvaluation_name, selfOtherGlobalEvaluation_desc);
    }

    struct Key : Data::KeyGroup<""> {
        // No keys for now
    };
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_FEATURE_TEST_HPP
