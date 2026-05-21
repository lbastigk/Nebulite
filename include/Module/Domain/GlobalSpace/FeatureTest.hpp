/**
 * @file FeatureTest.hpp
 * @brief Feature testing DomainModule for GlobalSpace.
 */

#ifndef MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP
#define MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/KeyGroup.hpp"
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::FeatureTest
 * @brief DomainModule for testing features in the GlobalSpace.
 */
class FeatureTest final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event testFuncTree() const ;
    static auto constexpr testFuncTree_name = "feature-test functree";
    static auto constexpr testFuncTree_desc = "Builds a funcTree with extra arguments and tests it\n"
        "Usage: feature-test functree\n";

    [[nodiscard]] Constants::Event selfOtherGlobalEvaluation() const ;
    static auto constexpr selfOtherGlobalEvaluation_name = "feature-test context-evaluation";
    static auto constexpr selfOtherGlobalEvaluation_desc = "Tests evaluation of self and other global variable access in one expression\n"
        "Usage: feature-test context-evaluation\n";

    [[nodiscard]] Constants::Event keyCombination(std::span<std::string const> const& args) const ;
    static auto constexpr keyCombination_name = "feature-test key-combination";
    static auto constexpr keyCombination_desc = "Tests key-combinations for the ScopedKey class.\n"
        "Usage: feature-test key-combination <key1> <key2>\n"
        "Using <empty> as argument will treated as an empty key.\n";

    //------------------------------------------
    // Categories

    static auto constexpr category_feature_test_name = "feature-test";
    static auto constexpr category_feature_test_desc = "Functions for testing features in the GlobalSpace\n"
        "Usage: feature-test <function>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit FeatureTest(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindCategory(category_feature_test_name, category_feature_test_desc);
        bindFunction(&FeatureTest::testFuncTree, testFuncTree_name, testFuncTree_desc);
        bindFunction(&FeatureTest::selfOtherGlobalEvaluation, selfOtherGlobalEvaluation_name, selfOtherGlobalEvaluation_desc);
        bindFunction(&FeatureTest::keyCombination, keyCombination_name, keyCombination_desc);
    }

    struct Key : Data::KeyGroup<""> {
        // No keys for now
    };
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP
