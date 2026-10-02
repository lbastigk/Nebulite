//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/Json.hpp"
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/FeatureTest.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"
#include "Nebulite/Utility/StringHandler.hpp"
#include "Nebulite/Utility/Testing.hpp"
#include "Nebulite/Utility/Time.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event FeatureTest::updateHook() {
    return Constants::Event::success;
}

// General

namespace {
class MathModifier {
public:
    static double add(std::span<std::string_view const> const args, double const input) {
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
    // Build a FuncTree
    Utility::Args::FuncTree<double, double> testTree("TestFuncTree", 0.0, std::numeric_limits<double>::quiet_NaN(), domain.capture);

    std::string_view constexpr addName = "add";
    std::string_view constexpr addDesc = "Adds all provided numbers to the input number.\nUsage: <name> add num1 num2 ... numN";

    // Using the DomainModule bindFunctionStatic to bind the add method, otherwise we would need to do some complex template/visit gymnastics here
    bindFunctionStatic(&testTree, &MathModifier::add, addName, addDesc);

    // Call the function
    auto constexpr funcCall = "<name> add 1.5 2.5 3.0";
    double const result = testTree.parseStr(funcCall, 0.0);
    domain.capture.log.println("FuncTree result for call '", funcCall, "': ", result);
    return Constants::Event::success;
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
        Data::Json selfAndOther;
        auto& self2 = selfAndOther.shareManagedScope("self.");
        auto& other2 = selfAndOther.shareManagedScope("other.");
        self2.set(key, 5);
        other2.set(key, 4);
        Interaction::Logic::Expression const expr("{self:testKey} {other:testKey} {global:testKey}");
        domain.capture.log.println(expr.eval({self2, other2, globalScope}));
    }
    return Constants::Event::success;
}

// Keys

Constants::Event FeatureTest::keyCombination(std::span<std::string_view const> const args) const {
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
    return Constants::Event::success;
}

Constants::Event FeatureTest::findParentKey(std::span<std::string_view const> const args) const {
    auto const key = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : "";
    domain.capture.log.println(Data::Json::findParentKey(key));
    return Constants::Event::success;
}

Constants::Event FeatureTest::largeFft(std::span<std::string_view const> const args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    auto size = Utility::Convert::Cast::String::to<std::size_t>(args[1]);
    if (!size.has_value()) {
        return Constants::StandardCapture::Warning::Functional::invalidArgument(domain.capture);
    }

    std::vector data(size.value(), 0.0);
    for (std::size_t i = 0; i < size.value(); ++i) {
        data[i] = static_cast<double>(i % 100); // Fill with some sample data
    }

    auto const start = Utility::Time::getTime();
    auto const result = Math::Fft::fft(data);
    auto const end = Utility::Time::getTime();

    domain.capture.log.println("FFT of size ", size.value(), " computed. Took ", end - start, " ms. First 10 results of ", result.size(), " total:");
    for (std::size_t i = 0; i < std::min(result.size(), static_cast<std::size_t>(10)); ++i) {
        domain.capture.log.println(result[i]);
    }
    if (result.size() != std::bit_ceil(size.value())) {
        domain.capture.error.println("FFT result size mismatch: expected ", std::bit_ceil(size.value()), " but got ", result.size());
        return Constants::Event::error;
    }
    return Constants::Event::success;
}

namespace {
std::array constexpr strings{
    // Simple whitespace tests
    "",
    " ",
    "  ",
    // Usual inputs
    "Hello world! These are split args.",
    "This  is  a  string  with  multiple  whitespaces",
    "This is  a   string   with  changing whitespaces",
    "ThisIsAStringWithoutWhitespaces",
    "one two three four five",
    "alpha beta gamma delta",
    "abababa",
    "repeat repeat repeat",
    "punctuation,! mixed.with; words?",
    " This is a string with a starting whitespace",
    "This is a string with an ending whitespace ",
    "  This is a string with two starting whitespaces",
    "This is a string with two ending whitespaces  ",
    "  alpha  beta   gamma  ",
    "Whitespaces.",
    "whitespaces.",
    " Whitespaces.",
    " whitespaces. ",
    "Whitespaces. ",
    "whitespaces. ",
    " Whitespaces. ",
    " whitespaces. ",
    // Some more tests with shortest words
    "a",
    " a",
    "a ",
    " a ",
    "  a",
    "a  ",
    "  a  ",
    "a   ",
    "   a",
    "   a   ",
    "a b",
    " a b ",
    "  a b  ",
    "   a b   ",
};
std::array constexpr containsValues = {
    "",
    " ",
    "  ",
    "   ",
    "This is",
    "This  is",
    "world! These",
    "These are split",
    "split args.",
    "multiple whitespaces",
    "changing whitespaces",
    "one two three",
    "two three four",
    "alpha beta",
    "beta gamma delta",
    "ababa",
    "babab",
    "repeat repeat",
    "punctuation,! mixed.with;",
    "not present",
    "a",
    "b",
    "a b",
    "a b ",
    " a b",
    " a b ",
    "  a b  ",
    "string",
    "String",
    "with",
    "With",
    "whitespaces",
    "whitespace",
    "whitespaces ",
    "whitespace ",
    "whitespaces. ",
    "whitespaces.",
    "ThisIsAStringWithoutWhitespaces",
};
} // namespace

Constants::Event FeatureTest::segmentedStringViewCompare() const {
    try{
        for (auto const* strRaw : strings) {
            auto str = std::string_view(strRaw);
            auto args = Utility::StringHandler::split(str, ' ');
            auto const ssv = Utility::Args::SegmentedStringView(args);

            Utility::assume(ssv == ssv, "Expected segmented string view to be equal to itself"); // NOLINT
            Utility::assume(ssv.characterCount() == str.size(), "Expected character count to match the source string: '", str, "'");
            Utility::assume(ssv == str, "Expected segmented string view operator== to match the source string: '", str, "'");
            Utility::assume(ssv.beginsWith(ssv), "Expected segmented string view to begin with itself (full)");
            Utility::assume(ssv.endsWith(ssv), "Expected segmented string view to end with itself (full)");

            // Compare substrings
            for (size_t i = 0; i < str.size(); ++i) {
                auto left = str.substr(0, i);
                auto right = str.substr(i);
                auto argsLeft = Utility::StringHandler::split(left, ' ');
                auto argsRight = Utility::StringHandler::split(right, ' ');

                // Compare against substrings
                Utility::assume(ssv.beginsWith(left), "Expected segmented string view to start with '", left, "'");
                Utility::assume(ssv.endsWith(right), "Expected segmented string view to end with '", right, "'");

                // Compare against another SegmentedStringView
                auto const ssvLeft = Utility::Args::SegmentedStringView(argsLeft);
                auto const ssvRight = Utility::Args::SegmentedStringView(argsRight);
                Utility::assume(ssv.beginsWith(ssvLeft), "Expected segmented string view to begin with itself until index ", i);
                Utility::assume(ssv.endsWith(ssvRight), "Expected segmented string view to end with itself from index ", i);

                // Additional checks, if possible
                if (!str.ends_with(left)) {
                    Utility::assume(!ssv.endsWith(ssvLeft), "Expected segmented string view to not end with the left substring");
                }
                if (!str.starts_with(right)) {
                    Utility::assume(!ssv.beginsWith(ssvRight), "Expected segmented string view to not begin with the right substring");
                }
            }
        }
        domain.capture.log.println("SegmentedStringView test passed.");
        return Constants::Event::success;
    } catch (std::runtime_error& e) {
        domain.capture.log.println("SegmentedStringView test failed: ", e.what());
        return Constants::Event::error;
    }
}

Constants::Event FeatureTest::segmentedStringViewContains() const {
    try {
        for (auto const* strRaw : strings) {
            auto str = std::string_view(strRaw);
            auto args = Utility::StringHandler::split(str, ' ');
            auto const ssv = Utility::Args::SegmentedStringView(args);

            // Check for contains values
            for (auto const* valRaw : containsValues) {
                // ssv-string_view compare
                auto val = std::string_view(valRaw);
                Utility::assume(
                    ssv.contains(val) == str.contains(val),
                    "Mismatch between contains methods ssv-string_view for value: '", val, "'. String is: '", str, "'"
                );

                // ssv-ssv compare
                auto valArgs = Utility::StringHandler::split(val, ' ');
                auto ssvVal = Utility::Args::SegmentedStringView(valArgs);
                Utility::assume(
                    ssv.contains(ssvVal) == str.contains(val),
                    "Mismatch between contains methods ssv-ssv for value: '", val, "'. String is: '", str, "'"
                );
            }

            // Each string must contain each substring of itself
            for (size_t i = 0; i < str.size(); ++i) {
                auto left = str.substr(0, i);
                auto right = str.substr(i);
                auto argsLeft = Utility::StringHandler::split(left, ' ');
                auto argsRight = Utility::StringHandler::split(right, ' ');

                // Compare against substrings
                Utility::assume(ssv.contains(left), "Expected segmented string view to contain '", left, "'");
                Utility::assume(ssv.contains(right), "Expected segmented string view to contain '", right, "'");

                // Compare against another SegmentedStringView
                auto const ssvLeft = Utility::Args::SegmentedStringView(argsLeft);
                auto const ssvRight = Utility::Args::SegmentedStringView(argsRight);
                Utility::assume(ssv.contains(ssvLeft), "Expected segmented string view to contain '", left, "' as ssv");
                Utility::assume(ssv.contains(ssvRight), "Expected segmented string view to contain '", right, "' as ssv");
            }
        }

        domain.capture.log.println("SegmentedStringView test passed.");
        return Constants::Event::success;
    } catch (std::runtime_error& e) {
        domain.capture.log.println("SegmentedStringView test failed: ", e.what());
        return Constants::Event::error;
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace
