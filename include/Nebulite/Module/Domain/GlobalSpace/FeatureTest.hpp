#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::FeatureTest
 * @brief DomainModule for exposing functionality to test features in the GlobalSpace domain.
 */
class FeatureTest final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // General

    [[nodiscard]] Constants::Event testFuncTree() const ;
    static auto constexpr testFuncTreeName = "feature-test functree";
    static auto constexpr testFuncTreeDesc = "Builds a funcTree with extra arguments and tests it\n"
        "Usage: feature-test functree\n";

    [[nodiscard]] Constants::Event selfOtherGlobalEvaluation() const ;
    static auto constexpr selfOtherGlobalEvaluationName = "feature-test context-evaluation";
    static auto constexpr selfOtherGlobalEvaluationDesc = "Tests evaluation of self and other global variable access in one expression\n"
        "Usage: feature-test context-evaluation\n";

    // Keys

    [[nodiscard]] Constants::Event keyCombination(std::span<std::string_view const> args) const ;
    static auto constexpr keyCombinationName = "feature-test key-combination";
    static auto constexpr keyCombinationDesc = "Tests key-combinations for the ScopedKey class.\n"
        "Usage: feature-test key-combination <key1> <key2>\n"
        "Using <empty> as argument will treated as an empty key.\n";

    [[nodiscard]] Constants::Event findParentKey(std::span<std::string_view const> args) const ;
    static auto constexpr findParentKeyName = "feature-test find-parent-key";
    static auto constexpr findParentKeyDesc = "Finds the parent key of a given key using the Json::findParentKey method.\n"
        "Usage: feature-test find-parent-key <key>\n"
        "Using no argument will treated as an empty key.\n";

    //------------------------------------------
    // Categories

    static auto constexpr categoryFeatureTestName = "feature-test";
    static auto constexpr categoryFeatureTestDesc = "Functions for testing features in the GlobalSpace\n"
        "Usage: feature-test <function>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit FeatureTest(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindCategory(categoryFeatureTestName, categoryFeatureTestDesc);

        // General
        bindFunction(&FeatureTest::testFuncTree, testFuncTreeName, testFuncTreeDesc);
        bindFunction(&FeatureTest::selfOtherGlobalEvaluation, selfOtherGlobalEvaluationName, selfOtherGlobalEvaluationDesc);

        // Keys
        bindFunction(&FeatureTest::keyCombination, keyCombinationName, keyCombinationDesc);
        bindFunction(&FeatureTest::findParentKey, findParentKeyName, findParentKeyDesc);
    }

    struct Key : Data::KeyGroup<""> {
        // No keys for now
    };
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP
