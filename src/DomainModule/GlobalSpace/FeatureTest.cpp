//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "DomainModule/GlobalSpace/FeatureTest.hpp"

#include "../../../include/Interaction/Execution/Domain.hpp"
#include "Utility/JSON.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error FeatureTest::update() {
    return Constants::ErrorTable::NONE();
}

class MathModifier{
public:
    double add(std::span<std::string const> const& args, double input) {
        double sum = input;
        // Add all arguments but the first (which is the function name)
        for (auto const& arg : args.subspan(1)) {
            try {
                sum += std::stod(arg);
            } catch (const std::invalid_argument&) {
                // Ignore invalid arguments
                return NAN;
            }
        }
        return sum;
    }
};

Constants::Error FeatureTest::testFuncTree(std::span<std::string const> const& args) {

    //
    Utility::Capture::cout() << "FeatureTest::testFuncTree called with " << args.size() << " arguments." << Utility::Capture::endl;
    for (auto const& arg : args){
        Utility::Capture::cout() << "  Arg: " << arg << Utility::Capture::endl;
    }

    // Create an instance of MathModifier so we can bind its method
    MathModifier mathModifier;

    // Build a FuncTree with extra argument JSON&
    Interaction::Execution::FuncTree<double, double> funcTree(
        "TestFuncTree",
        0.0,   // default return value
        NAN    // error return value
    );

    std::string const addName = "add";
    std::string const addDesc = "Adds all provided numbers to the input number.\nUsage: <name> add num1 num2 ... numN";

    // Using the DomainModule bindFunctionStatic to bind the add method, otherwise we would need to do some complex template/visit gymnastics here
    DomainModule::bindFunctionStatic(&funcTree, &mathModifier, &MathModifier::add, addName, &addDesc);

    std::string const funcCall = "<name> add 1.5 2.5 3.0";
    double const result = funcTree.parseStr(funcCall,0.0);
    Utility::Capture::cout() << "FuncTree result for call '" << funcCall << "': " << result << Utility::Capture::endl;

    return Constants::ErrorTable::NONE();

}
std::string const FeatureTest::testFuncTree_name = "feature-test functree";
std::string const FeatureTest::testFuncTree_desc = "Builds a funcTree with extra arguments and tests it";

//------------------------------------------
std::string const FeatureTest::category_feature_test_name = "feature-test";
std::string const FeatureTest::category_feature_test_desc = "Functions for testing features in the GlobalSpace";


}   // namespace Nebulite::DomainModule::GlobalSpace