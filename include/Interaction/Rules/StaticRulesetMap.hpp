/**
 * @file StaticRulesetMap.hpp
 * @brief This file contains predefined static rulesets for common interactions in the Nebulite engine.
 *        Compared to json-defined rulesets, static rulesets are hardcoded for performance and reliability.
 */

#ifndef INTERACTION_RULES_STATICRULESETMAP_HPP
#define INTERACTION_RULES_STATICRULESETMAP_HPP

//------------------------------------------
// Includes

// Standard library
#include <functional>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite

#include "Interaction/Context.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class RenderObject;
class GlobalSpace;
} // namespace Nebulite::Core

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Defining what a ruleset function looks like

using StaticRulesetFunction = std::function<void(const Context&, double** slf, double** otr)>;
using BaseListFunction = std::function<double**(Execution::Domain const&)>;

//------------------------------------------
// Defining a Ruleset Map where static rulesets can be looked up by name

// TODO: just like rmlinterface, we need a deletion tracker so we never use the global instance during destruction!
class StaticRulesetMap {
public:
    struct StaticRulesetWithMetadata {
        enum class Type : uint8_t {
            Local,
            Global,
            invalid
        } type = Type::invalid;
        std::string_view topic;
        std::string_view description;
        StaticRulesetFunction function = nullptr;
        BaseListFunction baseListFunc = nullptr;
    };

    StaticRulesetMap();

    ~StaticRulesetMap();

    StaticRulesetMap(StaticRulesetMap const&) = delete;
    StaticRulesetMap& operator=(StaticRulesetMap const&) = delete;
    StaticRulesetMap(StaticRulesetMap&&) = delete;
    StaticRulesetMap& operator=(StaticRulesetMap&&) = delete;

    /**
     * @brief Returns the instance of the StaticRulesetMap.
     * @return Reference to the StaticRulesetMap instance.
     */
    static StaticRulesetMap& getInstance();

    // Owned version of metadata for sharing info
    struct StaticRulesetMetadata {
        StaticRulesetWithMetadata::Type type;
        std::string topic;
        std::string description;
    };

    /**
     * @brief Get the metadata of all available static rulesets.
     * @return A vector of StaticRulesetMetadata for all available static rulesets.
     *         Each entry contains the type, topic, and description of a static ruleset.
     */
    static std::vector<StaticRulesetMetadata> getList();

    /**
     * @brief Retrieves a static ruleset function by name.
     * @param name The name of the static ruleset.
     * @return Pointer to the static ruleset function, with metadata.
     *         Returns an invalid entry if not found. Its type is `invalid`,
     *         and function pointer is `nullptr`.
     */
    StaticRulesetWithMetadata& getStaticRulesetByName(std::string const& name);

    /**
     * @brief Adds a static ruleset function to the map.
     * @param func Pointer to the static ruleset function, with metadata.
     */
    void bindStaticRuleset(StaticRulesetWithMetadata const& func);

private:
    absl::flat_hash_map<std::string, StaticRulesetWithMetadata> container;
    StaticRulesetWithMetadata invalidEntry;

    // List available rulesets
    void help(Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr helpName = "::help";
    static std::string_view constexpr helpDesc = "Lists all available static rulesets with their descriptions.";
    BaseListFunction const helpBaseListFunc = [](const Execution::Domain&) -> double** {return nullptr;};
};
} // namespace Nebulite::Interaction::Rules
#endif // INTERACTION_RULES_STATICRULESETMAP_HPP
