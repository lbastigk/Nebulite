/**
 * @file StaticRulesets.hpp
 * @brief This file contains predefined static rulesets for common interactions in the Nebulite engine.
 *        Compared to json-defined rulesets, static rulesets are hardcoded for performance and reliability.
 */

#ifndef NEBULITE_INTERACTION_RULES_STATIC_RULESETS_HPP
#define NEBULITE_INTERACTION_RULES_STATIC_RULESETS_HPP

//------------------------------------------
// Includes

// Standard library
#include <functional>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Nebulite.hpp"
#include "Interaction/Context.hpp"
#include "Construction/Initializer.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
class GlobalSpace;
}   // namespace Nebulite::Core

namespace Nebulite::Interaction::Execution {
class DomainBase;
}   // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Defining what a ruleset function looks like

using StaticRulesetFunction = std::function<void(const ContextBase&)>;

//------------------------------------------
// Defining a Ruleset Map where static rulesets can be looked up by name

class StaticRulesetMap {
public:
    struct StaticRuleSetWithMetaData {
        enum Type {
            Local,
            Global,
            invalid
        } type = invalid;
        std::string_view topic;
        std::string_view description;
        StaticRulesetFunction function = nullptr;
    };

    StaticRulesetMap() {
        invalidEntry = StaticRuleSetWithMetaData{
            StaticRuleSetWithMetaData::Type::invalid,
            "",
            "",
            nullptr
        };
        Construction::rulesetMapInit(this);
    }

    /**
     * @brief Returns the instance of the StaticRulesetMap.
     * @return Reference to the StaticRulesetMap instance.
     */
    static StaticRulesetMap& getInstance() {
        static StaticRulesetMap instance;
        return instance;
    }

    /**
     * @brief Retrieves a static ruleset function by name.
     * @param name The name of the static ruleset.
     * @return Pointer to the static ruleset function, with metadata.
     *         Returns an invalid entry if not found. Its type is `invalid`,
     *         and function pointer is `nullptr`.
     */
    StaticRuleSetWithMetaData& getStaticRulesetByName(std::string const& name) {
        if (container.contains(name)) {
            return container[name];
        }
        return invalidEntry;
    }

    /**
     * @brief Adds a static ruleset function to the map.
     * @param func Pointer to the static ruleset function, with metadata.
     */
    void bindStaticRuleset(StaticRuleSetWithMetaData const& func) {
        // Exit program if duplicate
        if (container.contains(func.topic)) {
            throw std::runtime_error("Duplicate static ruleset name: " + std::string(func.topic));
        }
        container[func.topic] = func;
    }

private:
    absl::flat_hash_map<std::string, StaticRuleSetWithMetaData> container;
    StaticRuleSetWithMetaData invalidEntry;
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_STATIC_RULESETS_HPP