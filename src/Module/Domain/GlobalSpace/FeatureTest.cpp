//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Interaction/Execution/FuncTree.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/FeatureTest.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"
#include "Nebulite/Utility/TypeConversion.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event FeatureTest::updateHook() {
    return Constants::Event::Success;
}

// General

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
        auto& self2 = selfAndOther.shareManagedScope("self.");
        auto& other2 = selfAndOther.shareManagedScope("other.");
        self2.set(key, 5);
        other2.set(key, 4);
        Interaction::Logic::Expression const expr("{self:testKey} {other:testKey} {global:testKey}");
        domain.capture.log.println(expr.eval({self2, other2, globalScope}));
    }
    return Constants::Event::Success;
}

// Keys

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

Constants::Event FeatureTest::findParentKey(std::span<std::string_view const> const& args) const {
    auto const key = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : "";
    domain.capture.log.println(Data::JSON::findParentKey(key));
    return Constants::Event::Success;
}

// FFT

namespace {

std::string formatComplex(std::complex<double> const& c) {
    if (c.imag() < 0) {
        return std::to_string(c.real()) + "-" + std::to_string(std::abs(c.imag())) + "i";
    }
    return std::to_string(c.real()) + "+" + std::to_string(c.imag()) + "i";
}

template <typename ResultRange>
void printResult(ResultRange const& result, Utility::IO::Capture& capture) {
    std::string str{};
    for (auto& c : result) {
        str += formatComplex(c);
        str += " ";
    }
    if (!str.empty()) {
        str.pop_back(); // Remove the last space
    }
    capture.log.println(str);
}

} // namespace

Constants::Event FeatureTest::fft(std::span<std::string_view const> const& args) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const sampleArgs = args.subspan(1)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;
    if (!sampleArgs) {
        domain.capture.log.println("Invalid argument format for FFT. All arguments must be valid numbers.");
        return Constants::Event::Error;
    }
    auto const result = Math::FFT::fft(sampleArgs.value());
    printResult(result, domain.capture);
    return Constants::Event::Success;
}

Constants::Event FeatureTest::ifft(std::span<std::string_view const> const& args) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() % 2 != 1) { // The first argument is the function name, so we need an even number of additional arguments for real-imaginary pairs
        domain.capture.log.println("Provide a list of real-imaginary pairs for the IFFT.");
        return Constants::Event::Error;
    }
    auto const sampleArgs = args.subspan(1)
        | std::views::chunk(2)
        | std::views::transform([](auto tuple) -> std::optional<std::complex<double>> {
            auto const real = Utility::TypeConversion::String::to<double>(tuple[0]);
            auto const imag = Utility::TypeConversion::String::to<double>(tuple[1]);
            if (!real || !imag) {
                return std::nullopt;
            }
            return std::complex<double>{real.value(), imag.value()};
        })
        | Utility::Ranges::collectOptional;
    if (!sampleArgs) {
        domain.capture.log.println("Invalid argument format for IFFT. All arguments must be valid numbers.");
        return Constants::Event::Error;
    }
    auto const result = Math::FFT::fftInverse(sampleArgs.value());
    printResult(result, domain.capture);
    return Constants::Event::Success;
}

Constants::Event FeatureTest::applyTransferFunction(std::span<std::string_view const> const& args) const {
    auto const numPos = std::ranges::find(args, std::string_view{"--num"});
    auto const denPos = std::ranges::find(args, std::string_view{"--den"});

    if (numPos == args.end() || denPos == args.end()) {
        domain.capture.log.println("Invalid argument format for apply-transfer-function.");
        return Constants::Event::Error;
    }
    if (numPos > denPos) {
        domain.capture.log.println("Invalid argument order for apply-transfer-function. '--num' should come before '--den'.");
        return Constants::Event::Error;
    }

    auto const numIndex = static_cast<std::size_t>(std::distance(args.begin(), numPos));
    auto const denIndex = static_cast<std::size_t>(std::distance(args.begin(), denPos));

    auto const sampleArgs = args.subspan(1, numIndex)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;
    auto const numArgs = args.subspan(numIndex + 1, denIndex)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;
    auto const denArgs = args.subspan(denIndex + 1)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;

    if (!sampleArgs || !numArgs || !denArgs) {
        domain.capture.log.println("Invalid argument format for apply-transfer-function. All arguments must be valid numbers.");
        return Constants::Event::Error;
    }

    auto const result = Math::FFT::applyTransferFunction(sampleArgs.value(), numArgs.value(), denArgs.value());
    printResult(result, domain.capture);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
