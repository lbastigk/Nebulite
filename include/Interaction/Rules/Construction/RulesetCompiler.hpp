/**
 * @file RulesetCompiler.hpp
 * @brief This file contains the static RulesetCompiler class.
 */

#ifndef NEBULITE_RULESET_COMPILER_HPP
#define NEBULITE_RULESET_COMPILER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules::Construction {
/**
 * @class RulesetCompiler
 * @brief Responsible for parsing compatible JSON documents into `Ruleset` structs.
 * @details A `Interaction::Execution::DomainBase` instance is required for context during parsing.
 *          It's field `invokes` holds the relevant invoke information.
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
    // A wrapper for all ruleset types, or none
    using AnyRuleset = std::variant<std::monostate, std::shared_ptr<StaticRuleset>, std::shared_ptr<JsonRuleset>>;

    /**
     * @brief Parses a JSON encoded set of Invoke Entries inside a Domain into Ruleset objects.
     * @param rulesetsGlobal The global Ruleset objects.
     * @param rulesetsLocal The local Ruleset objects.
     * @param self The Domain instance associated with the entries.
     */
    static void parse(
        std::vector<std::shared_ptr<Ruleset>>& rulesetsGlobal,
        std::vector<std::shared_ptr<Ruleset>>& rulesetsLocal,
        Interaction::Execution::DomainBase& self
        );

    /**
     * @brief Parses a single Ruleset from a JSON key inside a Domain or a static ruleset identifier.
     * @param identifier The key of the Ruleset entry in the Domain's JSON document, or a static ruleset identifier.
     * @param self The Domain instance associated with the entry.
     * @return An optional shared pointer to the parsed Ruleset object, or std::nullopt if parsing failed.
     */
    static std::optional<std::shared_ptr<Ruleset>> parseSingle(
        std::string const& identifier,
        Interaction::Execution::DomainBase& self
        );

private:
    /**
     * @brief Extracts function calls from a JSON entry document.
     * @param entryDoc The JSON document containing the entry.
     * @param Ruleset The Ruleset object to populate with function calls.
     * @param self The Domain instance associated with the entry.
     */
    static void getFunctionCalls(
        Core::JsonScope& entryDoc,
        JsonRuleset& Ruleset,
        Interaction::Execution::DomainBase const& self
        );

    /**
     * @brief Extract a single expression from a JSON entry document
     * @param assignmentExpr The assignment expression to populate.
     * @param entry The JSON entry document to extract the expression from.
     * @param index The index of the expression in the entry document.
     * @return True if the expression was successfully extracted, false otherwise.
     */
    static bool getExpression(
        Logic::Assignment& assignmentExpr,
        Core::JsonScope& entry,
        size_t const& index
        );

    /**
     * @brief Extracts all expressions from a JSON entry document.
     * @param Ruleset The Ruleset object to populate with expressions.
     * @param entry The JSON entry document to extract expressions from.
     * @param self The JSON document of context self.
     * @return True if the expressions were successfully extracted, false otherwise.
     */
    static bool getExpressions(std::shared_ptr<JsonRuleset> const& Ruleset, Core::JsonScope& entry, Core::JsonScope& self);

    /**
     * @brief Extracts a logical argument from a JSON entry document.
     * @param entry The JSON entry document to extract the argument from.
     * @return The extracted logical argument as a string.
     */
    static std::string getLogicalArg(Core::JsonScope& entry);

    /**
     * @brief Extracts a Ruleset object from a JSON entry document.
     * @param doc The JSON document containing the entry.
     * @param entry The JSON document to populate with the entry.
     * @param key The key of the entry in the document.
     * @return True if the Ruleset was successfully extracted, false otherwise.
     */
    static bool getJsonRuleset(
        Core::JsonScope& doc,
        Core::JsonScope& entry,
        Data::ScopedKey const& key
        );

    /**
     * @brief Extracts a Ruleset from a JSON document or static ruleset identifier.
     * @param doc The JSON document containing the entry.
     * @param key The key of the entry in the document.
     * @param self The Domain instance associated with the entry.
     * @return An optional shared pointer to the parsed Ruleset object, or std::monostate if parsing failed.
     */
    static AnyRuleset getRuleset(
        Core::JsonScope& doc,
        Data::ScopedKey const& key,
        Execution::DomainBase& self
        );

    /**
     * @brief Optimizes a Ruleset by linking direct target pointers.
     * @details Potentially modifying self and global by registering stable double pointers.
     * @param entry The Ruleset object to optimize.
     * @param self The Domain instance associated with the entries.
     */
    static void optimize(std::shared_ptr<JsonRuleset> const& entry, Core::JsonScope& self);

    /**
     * @brief Sets metadata in the object itself and in each Ruleset entry, including IDs, indices, and estimated computational cost.
     * @param self The Domain that owns the entries.
     * @param rulesetsLocal The local Ruleset objects.
     * @param rulesetsGlobal The global Ruleset objects.
     */
    static void setMetaData(
        Interaction::Execution::DomainBase& self,
        std::vector<std::shared_ptr<Nebulite::Interaction::Rules::Ruleset>> const& rulesetsLocal,
        std::vector<std::shared_ptr<Nebulite::Interaction::Rules::Ruleset>> const& rulesetsGlobal
        );
};
} // namespace Nebulite::Interaction::Rules::Construction
#endif // NEBULITE_RULESET_COMPILER_HPP
