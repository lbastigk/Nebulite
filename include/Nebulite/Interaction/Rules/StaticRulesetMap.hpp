#ifndef NEBULITE_INTERACTION_RULES_STATICRULESETMAP_HPP
#define NEBULITE_INTERACTION_RULES_STATICRULESETMAP_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"

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
// Defining a Ruleset Map where static rulesets can be looked up by name

class StaticRulesetMap {
public:
    struct StaticRulesetWithMetadata {
        Ruleset::Type type = Ruleset::Type::invalid;
        std::string_view topic;
        std::string_view description;
        void* instance;
        Ruleset::StaticRulesetFunction function = nullptr;
        Ruleset::BaseListFunction baseListFunc = nullptr;
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
        Ruleset::Type type;
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
    StaticRulesetWithMetadata& getStaticRulesetByName(std::string_view name);

    /**
     * @brief Adds a static ruleset function to the map.
     * @param func Pointer to the static ruleset function, with metadata.
     */
    void bindStaticRuleset(StaticRulesetWithMetadata const& func);

private:
    absl::flat_hash_map<std::string, StaticRulesetWithMetadata> container;
    StaticRulesetWithMetadata invalidEntry{
        .type=Ruleset::Type::invalid,
        .topic="",
        .description="",
        .instance=nullptr,
        .function=nullptr
    };

    // List available rulesets
    void help(Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr helpName = "::help";
    static std::string_view constexpr helpDesc = "Lists all available static rulesets with their descriptions.";
    Ruleset::BaseListFunction const helpBaseListFunc = [](const Execution::Domain&) -> double** {return nullptr;};
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_STATICRULESETMAP_HPP
