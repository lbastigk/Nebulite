/**
 * @file JsonRvalueTransformer.hpp
 * @brief Utility class for modifying JSON return value keys.
 *        JSON(key) -> value | transformation on value | transformation on value ... -> new value
 *        This allows for dynamic modification of JSON values during retrieval.
 */

#ifndef NEBULITE_UTILITY_JSON_RVALUE_TRANSFORMER_HPP
#define NEBULITE_UTILITY_JSON_RVALUE_TRANSFORMER_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string>
#include <vector>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Utility {
class JSON;
}

//------------------------------------------
namespace Nebulite::Utility {
class JsonRvalueTransformer {
private:
    /**
     * @brief The transformation tree is used to apply modifications to JSON values during getting
     *        if the key includes the pipe '|' character, we apply the transformations in sequence.
     *        Example: get<int>('MyKey.subKey|strLength|add 1')
     *                 will get the length of the string at MyKey.subKey and add 1 to it.
     *        Takes in a JSON* as argument to modify.
     *        Returns true on success, false on failure.
     */
    std::unique_ptr<Interaction::Execution::FuncTree<bool, JSON*>> transformationFuncTree;

    //------------------------------------------
    // Functions: Arithmetic

    bool add(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const addName;
    static std::string const addDesc;

    bool multiply(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const multiplyName;
    static std::string const multiplyDesc;

    bool mod(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const modName;
    static std::string const modDesc;

    bool pow(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const powName;
    static std::string const powDesc;

    //------------------------------------------
    // Functions: Array-related

    bool length(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const lengthName;
    static std::string const lengthDesc;

    bool at(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const atName;
    static std::string const atDesc;

    // TODO: push, pop, insert, remove, clear, etc. might be added later
    //       but first, add these functionalities to JSON class

    //------------------------------------------
    // Functions: Casting

    bool toInt(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const toIntName;
    static std::string const toIntDesc;

    bool toString(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const toStringName;
    static std::string const toStringDesc;

    //------------------------------------------
    // Functions: Debugging

    bool echo(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const echoName;
    static std::string const echoDesc;

    bool print(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const printName;
    static std::string const printDesc;

    //------------------------------------------
    // Functions: Type-related

    bool typeAsString(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const typeAsStringName;
    static std::string const typeAsStringDesc;

    bool typeAsNumber(std::span<std::string const> const& args, JSON* jsonDoc);
    static std::string const typeAsNumberName;
    static std::string const typeAsNumberDesc;

public:
    JsonRvalueTransformer();

    /**
     * @brief Binding helper using std::variant and std::
     * @param func The function to bind
     * @param name The name of the function
     * @param desc The description of the function
     */
    void bindTransformationFunction(typename Interaction::Execution::FuncTree<bool, JSON*>::template MemberMethod<JsonRvalueTransformer> func,std::string const& name,std::string const* desc) {
        transformationFuncTree->bindFunction(this,func,name,desc);
    }

    /**
     * @brief The key used to store and retrieve the value being modified.
     */
    static std::string const valueKey;

    /**
     * @brief Parses and applies JSON transformations from the given arguments.
     *        Example: get<int>("MyKey|add 5|multiply 2")
     *        Calls: parse(["add 5", "multiply 2"], inputDocument)
     *        Input document: {"value": 10}
     *        After applying the transformation, the document will be: {"value": 30}
     * @param args A list of arguments representing the transformations to apply.
     * @param jsonDoc The JSON document to modify. Should hold the value from the get operation
     *                in a specified key. On success, the modified value will be stored back in the same key.
     * @return true if the transformations were successfully applied, false otherwise.
     */
    bool parse(std::vector<std::string> const& args, JSON* jsonDoc);
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_JSON_RVALUE_TRANSFORMER_HPP