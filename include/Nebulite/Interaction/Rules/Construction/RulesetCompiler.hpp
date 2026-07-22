#ifndef NEBULITE_INTERACTION_RULES_CONSTRUCTION_RULESETCOMPILER_HPP
#define NEBULITE_INTERACTION_RULES_CONSTRUCTION_RULESETCOMPILER_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction::Rules {
class Ruleset;
class JsonRuleset;
class StaticRuleset;
} // namespace Nebulite::Interaction::Rules

//------------------------------------------
namespace Nebulite::Interaction::Rules::Construction {
/**
 * @class RulesetCompiler
 * @brief Responsible for parsing compatible JSON documents into `Ruleset` structs.
 * @todo The current ruleset integration should be flexible enough to integrate this into the Ruleset classes on construction?
 */
class RulesetCompiler {
public:
    // A wrapper for all ruleset types, or none
    using AnyRuleset = std::variant<
        std::monostate,
        std::shared_ptr<StaticRuleset>,
        std::shared_ptr<JsonRuleset>
    >;

    using RulesetVector = std::vector<std::shared_ptr<Ruleset>>;

    /**
     * @brief Parses a JSON encoded set of Invoke Entries inside a Domain into Ruleset objects.
     * @param rulesetsGlobal The global Ruleset objects.
     * @param rulesetsLocal The local Ruleset objects.
     * @param self The Domain instance associated with the entries.
     * @param rulesetArray The list of rulesets
     */
    static void parse(RulesetVector& rulesetsGlobal, RulesetVector& rulesetsLocal, Execution::Domain& self, Data::JsonScope const& rulesetArray);

    /**
     * @brief Parses a single Ruleset from a JSON key inside a Domain or a static ruleset identifier.
     * @param identifier The key of the Ruleset entry in the Domain's JSON document, or a static ruleset identifier.
     * @param self The Domain instance associated with the entry.
     * @return An optional shared pointer to the parsed Ruleset object, or std::nullopt if parsing failed.
     */
    static std::optional<std::shared_ptr<Ruleset>> parseSingle(std::string_view identifier, Execution::Domain& self);

private:
    /**
     * @brief Extracts function calls from a JSON entry document.
     * @param entryDoc The JSON document containing the entry.
     * @param Ruleset The Ruleset object to populate with function calls.
     */
    static void getFunctionCalls(Data::JsonScope const& entryDoc, JsonRuleset& Ruleset);

    /**
     * @brief Extracts all expressions from a JSON entry document.
     * @param Ruleset The Ruleset object to populate with expressions.
     * @param entry The JSON entry document to extract expressions from.
     * @return True if the expressions were successfully extracted, false otherwise.
     */
    static bool getAssignments(std::shared_ptr<JsonRuleset> const& Ruleset, Data::JsonScope const& entry);

    /**
     * @brief Extracts a logical argument from a JSON entry document.
     * @param entry The JSON entry document to extract the argument from.
     * @return The extracted logical argument as a string.
     */
    static std::string getCondition(Data::JsonScope const& entry);

    /**
     * @brief Extracts a Ruleset object from a JSON entry document.
     * @param doc The JSON document containing the entry.
     * @param entry The JSON document to populate with the entry.
     * @param key The key of the entry in the document.
     * @return True if the Ruleset was successfully extracted, false otherwise.
     */
    static bool getJsonRuleset(Data::JsonScope const& doc, Data::JsonScope& entry, Data::ScopedKeyView const& key);

    /**
     * @brief Extracts a Ruleset from a JSON document or static ruleset identifier.
     * @param doc The JSON document containing the entry.
     * @param key The key of the entry in the document.
     * @param self The Domain instance associated with the entry.
     * @return An optional shared pointer to the parsed Ruleset object, or std::monostate if parsing failed.
     */
    static AnyRuleset getRuleset(Data::JsonScope const& doc, Data::ScopedKeyView const& key, Execution::Domain& self);

    /**
     * @brief Optimizes a Ruleset by linking direct target pointers.
     * @details Potentially modifying self and global by registering stable double pointers.
     * @param entry The Ruleset object to optimize.
     * @param self The Domain instance associated with the entries.
     */
    static void optimize(std::shared_ptr<JsonRuleset> const& entry, Data::JsonScope& self);

    /**
     * @brief Sets metadata in the object.
     * @param rulesets The vector of Ruleset objects to set metadata for.
     */
    static void setMetaData(RulesetVector const& rulesets);
};
} // namespace Nebulite::Interaction::Rules::Construction
#endif // NEBULITE_INTERACTION_RULES_CONSTRUCTION_RULESETCOMPILER_HPP
