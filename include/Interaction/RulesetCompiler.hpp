/**
 * @file RulesetCompiler.hpp
 * @brief This file contains the static RulesetCompiler class.
 */

#ifndef NEBULITE_RULESET_COMPILER_HPP
#define NEBULITE_RULESET_COMPILER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Core/RenderObject.hpp"
#include "Data/DocumentCache.hpp"
#include "Data/JSON.hpp"
#include "Interaction/Logic/Assignment.hpp"

//------------------------------------------
namespace Nebulite::Interaction {
/**
 * @class RulesetCompiler
 * @brief Responsible for parsing compatible JSON documents into `Ruleset` structs.
 *        A `Core::RenderObject` instance is required for context during parsing.
 *        It's field `invokes` holds the relevant invoke information.
 * @todo Idea for Invoke ruleset overwrites:
 *       In addition, add the field "overwrites" to the JSON doc.
 *       Then, on parsing, the overwrites are applied:
 *       $(overwrites.key1) would be replaced by "value1"
 *       If, however, an overwrite is not found:
 *       $(overwrites.key3) would be replaced by $(global.key3)
 *       This allows us to flexibly overwrite values in the invoke without changing the original JSON file.
 *       Also, the behavior is well-defined, as it defaults to the global value if no overwrite is defined.
 *       Note: retrieval of overwrites in a type object might be difficult. Instead, perhaps:
 *       myInvoke.jsonc|push-back overwrites 'key1 -> value1'
 *       Example JSON:
 *       ```json
 *       {
 *       "overwrites": {
 *           "key1": "value1",
 *           "key2": "value2"
 *       }
 *       ```
 *       This makes subkey-overwrites easier to parse, e.g.: `"overwrites" [ "physics.G -> 9.81" ]`
 *       turns an `$(overwrites.physics.G)` into `9.81` and
 *       defaults to `{global.physics.G}` if not overwritten.
 */
class RulesetCompiler {
public:
    /**
     * @brief Parses a JSON encoded set of Invoke Entries inside a RenderObject into Ruleset objects.
     * @param entries_global The global Ruleset objects.
     * @param entries_local The local Ruleset objects.
     * @param self The RenderObject instance associated with the entries.
     * @param docCache The DocumentCache instance to use for parsing expressions.
     */
    static void parse(
        std::vector<std::shared_ptr<Ruleset>>& entries_global,
        std::vector<std::shared_ptr<Ruleset>>& entries_local,
        Core::RenderObject* self
        );

private:
    /**
     * @brief Extracts function calls from a JSON entry document.
     * @param entryDoc The JSON document containing the entry.
     * @param Ruleset The Ruleset object to populate with function calls.
     * @param self The RenderObject instance associated with the entry.
     */
    static void getFunctionCalls(
        Data::JSON& entryDoc,
        Ruleset& Ruleset,
        Core::RenderObject const* self
        );

    /**
     * @brief Extract a single expression from a JSON entry document
     * @param assignmentExpr The assignment expression to populate.
     * @param entry The JSON entry document to extract the expression from.
     * @param index The index of the expression in the entry document.
     * 
     * @return True if the expression was successfully extracted, false otherwise.
     */
    static bool getExpression(
        Logic::Assignment& assignmentExpr,
        Data::JSON& entry,
        size_t const& index
        );

    /**
     * @brief Extracts all expressions from a JSON entry document.
     * @param Ruleset The Ruleset object to populate with expressions.
     * @param entry The JSON entry document to extract expressions from.
     * @return True if the expressions were successfully extracted, false otherwise.
     */
    static bool getExpressions(std::shared_ptr<Ruleset> const& Ruleset, Data::JSON* entry);

    /**
     * @brief Extracts a logical argument from a JSON entry document.
     * @param entry The JSON entry document to extract the argument from.
     * @return The extracted logical argument as a string.
     */
    static std::string getLogicalArg(Data::JSON& entry);

    /**
     * @brief Extracts an Ruleset object from a JSON entry document.
     * @param doc The JSON document containing the entry.
     * @param entry The JSON document to populate with the entry.
     * @param index The index of the entry in the document.
     * @return True if the Ruleset was successfully extracted, false otherwise.
     */
    static bool getRuleset(
        Data::JSON& doc,
        Data::JSON& entry,
        size_t const& index
        );

    /**
     * @brief Optimizes the parsed entries by linking direct target pointers.
     *        Potentially modifying self and global by registering stable double pointers.
     * @param entries The Ruleset objects to optimize.
     * @param self The RenderObject instance associated with the entries.
     */
    static void optimizeParsedEntries(
        std::vector<std::shared_ptr<Ruleset>> const& entries,
        Data::JSON* self
        );

    /**
     * @brief List of operations that are considered numeric and thus eligible for direct pointer assignment.
     * @note Any new numeric operation must be added here to benefit from optimization techniques in the Invoke class.
     */
    inline static std::vector<Logic::Assignment::Operation> const numeric_operations = {
        Logic::Assignment::Operation::set,
        Logic::Assignment::Operation::add,
        Logic::Assignment::Operation::multiply
    };
};
} // namespace Nebulite::Interaction
#endif // NEBULITE_RULESET_COMPILER_HPP