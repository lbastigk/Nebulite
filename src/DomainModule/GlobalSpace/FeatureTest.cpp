//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "DomainModule/GlobalSpace/FeatureTest.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error FeatureTest::update() {
    return Constants::ErrorTable::NONE();
}

class MathModifier {
public:
    static double add(std::span<std::string const> const& args, double const input) {
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

Constants::Error FeatureTest::testFuncTree(std::span<std::string const> const& /*args*/) {
    // Build a FuncTree with extra argument JSON&
    Interaction::Execution::FuncTree<double, double> testTree("TestFuncTree", 0.0, std::numeric_limits<double>::quiet_NaN());

    std::string_view constexpr addName = "add";
    std::string_view constexpr addDesc = "Adds all provided numbers to the input number.\nUsage: <name> add num1 num2 ... numN";

    // Using the DomainModule bindFunctionStatic to bind the add method, otherwise we would need to do some complex template/visit gymnastics here
    bindFunctionStatic(&testTree, &MathModifier::add, addName, addDesc);

    // Call the function
    std::string const funcCall = "<name> add 1.5 2.5 3.0";
    double const result = testTree.parseStr(funcCall, 0.0);
    Log::println("FuncTree result for call '", funcCall, "': ", result);
    return Constants::ErrorTable::NONE();
}

Constants::Error FeatureTest::selfOtherGlobalEvaluation() const {
    Data::ScopedKey const key("testKey");
    auto const token = getDomainModuleAccessToken(*this);
    auto& globalScope = Global::shareScopeBase(token);
    globalScope.set(key, 3);

    // Test 1: separate scopeBase
    {
        Data::JsonScopeBase self1;
        self1.set(key, 1);
        Data::JsonScopeBase other1;
        other1.set(key, 2);
        Interaction::Logic::Expression const expr("{self.testKey} {other.testKey} {global.testKey}",self1);
        Log::println(expr.eval(other1));
    }

    // Test 2: share managed scopeBase
    {
        Data::JSON selfAndOther;
        auto& self2 = selfAndOther.shareManagedScopeBase("self.");
        auto& other2 = selfAndOther.shareManagedScopeBase("other.");
        self2.set(key, 5);
        other2.set(key, 4);
        Interaction::Logic::Expression const expr("{self.testKey} {other.testKey} {global.testKey}",self2);
        Log::println(expr.eval(other2));
    }
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::GlobalSpace
