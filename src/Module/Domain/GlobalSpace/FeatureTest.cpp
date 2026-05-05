//------------------------------------------
// Includes

// Nebulite
#include "Module/Domain/GlobalSpace/FeatureTest.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event FeatureTest::updateHook() {
    return Constants::Event::Success;
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

Constants::Event FeatureTest::keyCombination(std::span<std::string const> const& args) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 3) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    std::string const key1 = args[1] == "<empty>" ? "" : args[1];
    std::string const key2 = args[2] == "<empty>" ? "" : args[2];
    auto const key = Data::ScopedKey(key1).addMember(key2);
    domain.capture.log.println(key.view().toString());
    return Constants::Event::Success;
}

namespace {
struct SensorReading {
    double convertedValue = -271.3;
    uint32_t rawValue = 0;

    bool operator==(const SensorReading& rhs) const {
        return rawValue == rhs.rawValue && Math::isEqual(convertedValue, rhs.convertedValue);
    }
};
struct ComputerInfo {
    SensorReading cpu;
    SensorReading gpu;
    SensorReading ram;
    std::array<int, 6> HWID = {1, 2, 3, 4, 5, 6};

    bool operator==(const ComputerInfo& rhs) const {
        bool result = true;
        result &= cpu == rhs.cpu;
        result &= gpu == rhs.gpu;
        result &= ram == rhs.ram;
        result &= HWID == rhs.HWID;
        return result;
    }
};
struct StructWithArray {
    int normalValue = 123;
    std::array<double, 3> rawValue = {7.11, 3.141, 2.};
};
} // namespace

Constants::Event FeatureTest::reflection() const {
    domain.capture.log.println("Testing reflection capability");
    domain.capture.log.println("Struct1 -> JSON -> Struct2, then checking for equality");

    // 1.) struct -> JSON
    Data::JsonScope info1;
    ComputerInfo infoObj1{
        .cpu = {.convertedValue = 55.5, .rawValue = 1023},
        .gpu = {.convertedValue = 65.2, .rawValue = 2047},
        .ram = {.convertedValue = 16.0, .rawValue = 4095},
        .HWID = {42, 43, 44, 45, 46, 47}
    };
    info1.setObject<ComputerInfo>(infoObj1);
    domain.capture.log.println("Output:");
    domain.capture.log.println(info1.serialize());

    // 2.) json -> struct
    // Making sure that info2 is a fresh JSON, in case there are some caching issues
    // That part is tested separately
    Data::JsonScope info2;
    info2.deserialize(info1.serialize());
    if (ComputerInfo infoObj2 = info2.getObject<ComputerInfo>(); infoObj1 != infoObj2) {
        domain.capture.error.println("Objects are not equal!");
        return Constants::Event::Error;
    }
    domain.capture.log.println("Objects are equal!");
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
