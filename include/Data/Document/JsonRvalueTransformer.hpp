/**
 * @file JsonRvalueTransformer.hpp
 * @brief Data class for modifying JSON return value keys.
 *        JSON(key) -> value | transformation on value | transformation on value ... -> new value
 *        This allows for dynamic modification of JSON values during retrieval.
 */

#ifndef NEBULITE_DATA_JSON_RVALUE_TRANSFORMER_HPP
#define NEBULITE_DATA_JSON_RVALUE_TRANSFORMER_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string>
#include <vector>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Data {
class JSON;
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
class JsonRvalueTransformer {
private:
    /**
     * @brief The transformation tree is used to apply modifications to JSON values during getting
     * @details if the key includes the pipe '|' character, we apply the transformations in sequence.
     *          Example: get<int>('MyKey.subKey|strLength|add 1')
     *                   will get the length of the string at MyKey.subKey and add 1 to it.
     *          Takes in a JSON* as argument to modify.
     *          Returns true on success, false on failure.
     */
    std::unique_ptr<Interaction::Execution::FuncTree<bool, JsonScope*>> transformationFuncTree;

    //------------------------------------------
    // Functions: Arithmetic
    // TODO: For all arithmetic functions: using multiple numbers should output array?

    bool add(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr addName = "add";
    static std::string_view constexpr addDesc = "Adds a numeric value to the current JSON value.\n"
        "Usage: |add <number1> <number2> ... -> {number}\n";

    bool mod(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr modName = "mod";
    static std::string_view constexpr modDesc = "Calculates the modulo of the current JSON value by a numeric value.\n"
        "Usage: |mod <number> -> {number}\n";

    bool multiply(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr multiplyName = "mul";
    static std::string_view constexpr multiplyDesc = "Multiplies the current JSON value by a numeric value.\n"
        "Usage: |multiply <number1> <number2> ...\n";

    bool pow(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr powName = "pow";
    static std::string_view constexpr powDesc = "Raises the current JSON value to the power of a numeric value.\n"
        "Usage: |pow <exponent> -> {number}\n";

    //------------------------------------------
    // Functions: Array-related

    bool ensureArray(JsonScope* jsonDoc);
    static std::string_view constexpr ensureArrayName = "ensureArray";
    static std::string_view constexpr ensureArrayDesc = "Ensures the current JSON value is an array.\n"
        "If the current value is not an array, it is wrapped into a single-element array.\n"
        "Usage: |ensureArray -> {array}\n";

    bool at(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr atName = "at";
    static std::string_view constexpr atDesc = "Gets the element at the specified index from the array in the current JSON value.\n"
        "If the index is out of bounds, the transformation fails.\n"
        "Usage: |at <index> -> {value}\n";

    bool length(JsonScope* jsonDoc);
    static std::string_view constexpr lengthName = "length";
    static std::string_view constexpr lengthDesc = "Gets the length of the array in the current JSON value.\n"
        "Usage: |length -> {number}\n";

    bool reverse(JsonScope* jsonDoc);
    static std::string_view constexpr reverseName = "reverse";
    static std::string_view constexpr reverseDesc = "Reverses the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |reverse -> {array}\n";

    bool first(JsonScope* jsonDoc);
    static std::string_view constexpr firstName = "first";
    static std::string_view constexpr firstDesc = "Gets the first element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |first -> {value}\n";

    bool last(JsonScope* jsonDoc);
    static std::string_view constexpr lastName = "last";
    static std::string_view constexpr lastDesc = "Gets the last element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |last -> {value}\n";

    // TODO: push, pop, insert, remove, clear, etc.
    //       but first, add these functionalities to JSON class

    // TODO: dedupe, subarray, sort, sortby, pick, omit

    //------------------------------------------
    // Functions: Assertions

    bool assertNonEmpty(JsonScope* jsonDoc);
    static std::string_view constexpr assertNonEmptyName = "assertNonEmpty";
    static std::string_view constexpr assertNonEmptyDesc = "Asserts that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails and the program exits\n"
        "Usage: |assertNonEmpty -> {value,<Exception thrown if empty>}\n";

    //------------------------------------------
    // Functions: Casting

    bool toInt(JsonScope* jsonDoc);
    static std::string_view constexpr toIntName = "toInt";
    static std::string_view constexpr toIntDesc = "Converts the current JSON value to an integer.\n"
        "Never fails, defaults to 0 if the provided value is non-numeric.\n"
        "Usage: |toInt -> {number}\n";

    // TODO: complicated casting for string to anything via serialization
    bool toString(JsonScope* jsonDoc);
    static std::string_view constexpr toStringName = "toString";
    static std::string_view constexpr toStringDesc = "Converts the current JSON value to a string.\n"
        "Never fails, defaults to an empty string if no conversion is possible.\n"
        "Usage: |toString -> {string}\n";

    bool toBool(JsonScope* jsonDoc);
    static std::string_view constexpr toBoolName = "toBool";
    static std::string_view constexpr toBoolDesc = "Converts the current JSON value to a boolean.\n"
        "Usage: |toBool -> {bool}\n"
        "'true' values: true, 1, '1', 'true', 'yes', 'on' (case-insensitive)\n"
        "'false' values: false, 0, '0', 'false', 'no', 'off' (case-insensitive)\n"
        "Any other value defaults to false.\n";

    bool toDouble(JsonScope* jsonDoc);
    static std::string_view constexpr toDoubleName = "toDouble";
    static std::string_view constexpr toDoubleDesc = "Converts the current JSON value to a double.\n"
        "Usage: |toDouble -> {number}\n"
        "Non-numeric values default to 0.0.\n";

    bool toBoolString(JsonScope* jsonDoc);
    static std::string_view constexpr toBoolStringName = "toBoolString";
    static std::string_view constexpr toBoolStringDesc = "Converts the current JSON value to a boolean string.\n"
        "Usage: |toBoolString -> {string}\n"
        "Either 'true' or 'false'\n";

    //------------------------------------------
    // Functions: Collection

    // TODO: filter
    //       glob-patterns would be useful here
    //       but first, we need to implement object key retrieval in JSON class
    //bool filter(std::span<std::string const> const& args, JsonScope* jsonDoc);
    //static std::string_view constexpr filterName;
    //static std::string_view constexpr filterDesc;

    bool map(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr mapName = "map";
    static std::string_view constexpr mapDesc = "Applies a mapping function to each element in the array of the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |map <function> -> {array}\n";

    bool get(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr getName = "get";
    static std::string_view constexpr getDesc = "Gets the value at the specified key from the current JSON object.\n"
        "Usage: |get <key> -> {value}\n";

    bool getMultiple(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr getMultipleName = "getMultiple";
    static std::string_view constexpr getMultipleDesc = "Gets multiple values at the specified keys from the current JSON object.\n"
        "Usage: |getMultiple <key1> <key2> ... -> {array of values}\n";

    //------------------------------------------
    // Functions: Debugging

    bool echo(std::span<std::string const> const& args);
    static std::string_view constexpr echoName = "echo";
    static std::string_view constexpr echoDesc = "Echoes the provided arguments to the console, with newline.\n"
        "Usage: |echo <arg1> <arg2> -> {unchanged-json}\n";

    bool print(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr printName = "print";
    static std::string_view constexpr printDesc = "Prints the current JSON value to the console.\n"
        "Usage: |print -> {unchanged-json}\n";

    //------------------------------------------
    // Functions: Domain

    bool nebs(std::span<std::string const> const& args, JsonScope* jsonDoc);
    static std::string_view constexpr nebsName = "nebs";
    static std::string_view constexpr nebsDesc = "Parses a Nebulite Script command on the JSON\n"
        "Usage: |nebs <command> -> {value}\n";

    //------------------------------------------
    // Functions: Statistics

    // TODO: total, median, average, stddev, min, max, etc.
    //       Make sure to allow for non-numeric values if appropriate

    //------------------------------------------
    // Functions: String

    // TODO: trim, upper, lower, substring, replace, etc.

    //------------------------------------------
    // Functions: Type-related

    bool typeAsNumber(JsonScope* jsonDoc);
    static std::string_view constexpr typeAsNumberName = "typeAsNumber";
    static std::string_view constexpr typeAsNumberDesc = "Converts the current JSON type value to a number.\n"
        "Usage: |typeAsNumber -> {number}"
        "where the number reflects the enum value JSON::KeyType.\n";

    bool typeAsString(JsonScope* jsonDoc);
    static std::string_view constexpr typeAsStringName = "typeAsString";
    static std::string_view constexpr typeAsStringDesc = "Converts the current JSON type value to a string.\n"
        "Usage: |typeAsString -> {value,array,object}\n";

    // TODO: exists, isType <name>

public:
    JsonRvalueTransformer();

    /**
     * @brief The key used to store and retrieve the value being modified.
     * @details Uses an empty string as key, so the entire JSON document is the value used.
     */
    static std::string_view constexpr valueKey = "";

    /**
     * @brief Binding helper using std::variant and std::
     * @param functionPtr The function to bind
     * @param name The name of the function
     * @param desc The description of the function
     */
    template <typename Func>
    void bindTransformationFunction(
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& desc
    ) {
        Interaction::Execution::DomainModuleBase::bindFunctionStatic(
            transformationFuncTree.get(),
            functionPtr,
            name,
            desc
        );
    }

    /**
     * @brief Parses and applies JSON transformations from the given arguments.
     * @details Example: get<int>("MyKey|add 5|multiply 2")
     *          Calls: parse(["add 5", "multiply 2"], inputDocument)
     *          Input document: {"value": 10}
     *          After applying the transformation, the document will be: {"value": 30}
     * @param args A list of arguments representing the transformations to apply.
     * @param jsonDoc The JSON document to modify. Should hold the value from the get-operation
     *                in a specified key. On success, the modified value will be stored back in the same key.
     * @return true if the transformations were successfully applied, false otherwise.
     */
    bool parse(std::vector<std::string> const& args, JsonScope* jsonDoc);
    bool parse(std::vector<std::string> const& args, JSON* jsonDoc);
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_JSON_RVALUE_TRANSFORMER_HPP
