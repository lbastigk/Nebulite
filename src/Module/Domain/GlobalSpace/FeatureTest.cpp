//------------------------------------------
// Includes

// Standard library
#include <limits>
#include <span>
#include <stdexcept>
#include <string>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Module/Domain/GlobalSpace/FeatureTest.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event FeatureTest::updateHook() {
    return Constants::Event::Success;
}

namespace {
class MathModifier {
public:
    static double add(std::span<std::string_view const> const& args, double const input) {
        double sum = input;
        // Add all arguments but the first (which is the function name)
        for (auto const& arg : args.subspan(1)) {
            try {
                sum += std::stod(std::string(arg));
            } catch (std::invalid_argument const&) {
                // Ignore invalid arguments
                return std::numeric_limits<double>::quiet_NaN();
            }
        }
        return sum;
    }
};
} // namespace

Constants::Event FeatureTest::testFuncTree() const {
    // Build a FuncTree with extra argument JSON&
    Interaction::Execution::FuncTree<double, double> testTree("TestFuncTree", 0.0, std::numeric_limits<double>::quiet_NaN(), domain.capture);

    std::string_view constexpr addName = "add";
    std::string_view constexpr addDesc = "Adds all provided numbers to the input number.\nUsage: <name> add num1 num2 ... numN";

    // Using the DomainModule bindFunctionStatic to bind the add method, otherwise we would need to do some complex template/visit gymnastics here
    bindFunctionStatic(&testTree, &MathModifier::add, addName, addDesc);

    // Call the function
    auto constexpr funcCall = "<name> add 1.5 2.5 3.0";
    double const result = testTree.parseStr(funcCall, 0.0);
    domain.capture.log.println("FuncTree result for call '", funcCall, "': ", result);
    return Constants::Event::Success;
}

Constants::Event FeatureTest::selfOtherGlobalEvaluation() const {
    Data::ScopedKey const key("testKey");
    auto globalScope = Data::JsonScope();
    globalScope.set(key, 3);

    // Test 1: separate scopeBase
    {
        Data::JsonScope self1;
        self1.set(key, 1);
        Data::JsonScope other1;
        other1.set(key, 2);
        Interaction::Logic::Expression const expr("{self:testKey} {other:testKey} {global:testKey}");
        domain.capture.log.println(expr.eval({self1, other1, globalScope}));
    }

    // Test 2: share managed scopeBase
    {
        Data::JSON selfAndOther;
        auto& self2 = selfAndOther.shareManagedScopeBase("self.");
        auto& other2 = selfAndOther.shareManagedScopeBase("other.");
        self2.set(key, 5);
        other2.set(key, 4);
        Interaction::Logic::Expression const expr("{self:testKey} {other:testKey} {global:testKey}");
        domain.capture.log.println(expr.eval({self2, other2, globalScope}));
    }
    return Constants::Event::Success;
}

Constants::Event FeatureTest::keyCombination(std::span<std::string_view const> const& args) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    auto const key1 = args[1] == "<empty>" ? "" : args[1];
    auto const key2 = args[2] == "<empty>" ? "" : args[2];
    auto const key = Data::ScopedKey(key1).addMember(key2);
    domain.capture.log.println(key.view().toString());
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
