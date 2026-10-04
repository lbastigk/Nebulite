#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Utility/Args/CmdArgs.hpp"

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

    [[nodiscard]] Constants::Event keyCombination(Utility::Args::SSV const& args) const ;
    static auto constexpr keyCombinationName = "feature-test key-combination";
    static auto constexpr keyCombinationDesc = "Tests key-combinations for the ScopedKey class.\n"
        "Usage: feature-test key-combination <key1> <key2>\n"
        "Using <empty> as argument will treated as an empty key.\n";

    [[nodiscard]] Constants::Event findParentKey(Utility::Args::SSV const& args) const ;
    static auto constexpr findParentKeyName = "feature-test find-parent-key";
    static auto constexpr findParentKeyDesc = "Finds the parent key of a given key using the Json::findParentKey method.\n"
        "Usage: feature-test find-parent-key <key>\n"
        "Using no argument will treated as an empty key.\n";

    // Benchmarks

    [[nodiscard]] Constants::Event largeFft(Utility::Args::SSV const& args) const ;
    static auto constexpr largeFftName = "feature-test large-fft";
    static auto constexpr largeFftDesc = "Tests the FFT implementation with a large dataset.\n"
        "Usage: feature-test large-fft <size>\n";

    // Segmented String view

    [[nodiscard]] Constants::Event segmentedStringViewCompare() const ;
    static auto constexpr segmentedStringViewCompareName = "feature-test segmented-string-view compare";
    static auto constexpr segmentedStringViewCompareDesc = "Segmented string view comparison tests.\n"
        "Prints the message 'SegmentedStringView test passed.' or 'SegmentedStringView test failed: <reason>'"
        "Usage: feature-test segmented-string-view compare\n";

    [[nodiscard]] Constants::Event segmentedStringViewContains() const ;
    static auto constexpr segmentedStringViewContainsName = "feature-test segmented-string-view contains";
    static auto constexpr segmentedStringViewContainsDesc = "Tests the contains method of the SegmentedStringView class.\n"
        "Prints the message 'SegmentedStringView test passed.' or 'SegmentedStringView test failed: <reason>'"
        "Usage: feature-test segmented-string-view contains\n";

    [[nodiscard]] Constants::Event segmentedStringViewBenchmark(Utility::Args::SSV const& args) const ;
    static auto constexpr segmentedStringViewBenchmarkName = "feature-test segmented-string-view benchmark";
    static auto constexpr segmentedStringViewBenchmarkDesc = "Tests benchmark for segmented string-view, using compare and contains methods.\n"
        "Prints the amount of milliseconds the test took."
        "Usage: feature-test segmented-string-view <size>\n";

    //------------------------------------------
    // Categories

    static auto constexpr categoryFeatureTestName = "feature-test";
    static auto constexpr categoryFeatureTestDesc = "Functions for testing features in the GlobalSpace\n"
        "Usage: feature-test <function>\n";

    static auto constexpr categoryFeatureTestSegmentedStringViewName = "feature-test segmented-string-view";
    static auto constexpr categoryFeatureTestSegmentedStringViewDesc = "Functions for testing segmented string view functionality.\n";

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

        // Benchmarks
        bindFunction(&FeatureTest::largeFft, largeFftName, largeFftDesc);

        // Segmented String View
        bindCategory(categoryFeatureTestSegmentedStringViewName, categoryFeatureTestSegmentedStringViewDesc);
        bindFunction(&FeatureTest::segmentedStringViewCompare, segmentedStringViewCompareName, segmentedStringViewCompareDesc);
        bindFunction(&FeatureTest::segmentedStringViewContains, segmentedStringViewContainsName, segmentedStringViewContainsDesc);
        bindFunction(&FeatureTest::segmentedStringViewBenchmark, segmentedStringViewBenchmarkName, segmentedStringViewBenchmarkDesc);
    }

    struct Key : Data::KeyGroup<""> {
        // No keys for now
    };
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FEATURETEST_HPP
