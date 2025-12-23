//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "DomainModule/GlobalSpace/FeatureTest.hpp"
#include "Data/JSON.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error FeatureTest::update() {
    return Constants::ErrorTable::NONE();
}

class MathModifier {
public:
    double add(std::span<std::string const> const& args, double input) {
        double sum = input;
        // Add all arguments but the first (which is the function name)
        for (auto const& arg : args.subspan(1)) {
            try {
                sum += std::stod(arg);
            } catch (std::invalid_argument const&) {
                // Ignore invalid arguments
                return std::numeric_limits<double>::quiet_NaN();
            }
        }
        return sum;
    }
};

Constants::Error FeatureTest::testFuncTree(std::span<std::string const> const& args) {
    // Create an instance of MathModifier so we can bind its method
    MathModifier mathModifier;

    // Build a FuncTree with extra argument JSON&
    Interaction::Execution::FuncTree<double, double> testTree("TestFuncTree", 0.0, std::numeric_limits<double>::quiet_NaN());

    std::string const addName = "add";
    std::string const addDesc = "Adds all provided numbers to the input number.\nUsage: <name> add num1 num2 ... numN";

    // Using the DomainModule bindFunctionStatic to bind the add method, otherwise we would need to do some complex template/visit gymnastics here
    DomainModule::bindFunctionStatic(&testTree, &mathModifier, &MathModifier::add, addName, &addDesc);

    // Call the function
    std::string const funcCall = "<name> add 1.5 2.5 3.0";
    double const result = testTree.parseStr(funcCall, 0.0);
    Nebulite::cout() << "FuncTree result for call '" << funcCall << "': " << result << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

std::string const FeatureTest::testFuncTree_name = "feature-test functree";
std::string const FeatureTest::testFuncTree_desc = "Builds a funcTree with extra arguments and tests it";

//------------------------------------------
std::string const FeatureTest::category_feature_test_name = "feature-test";
std::string const FeatureTest::category_feature_test_desc = "Functions for testing features in the GlobalSpace";


} // namespace Nebulite::DomainModule::GlobalSpace
