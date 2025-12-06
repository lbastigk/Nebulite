/**
 * @file StaticRulesets.hpp
 * @brief This file contains predefined static rulesets for common interactions in the Nebulite engine.
 *        Compared to json-defined rulesets, static rulesets are hardcoded for performance and reliability.
 */

#ifndef NEBULITE_INTERACTION_RULES_STATIC_RULESETS_HPP
#define NEBULITE_INTERACTION_RULES_STATIC_RULESETS_HPP

//------------------------------------------
// Includes
#include <functional>

// Standard library

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Nebulite.hpp"
#include "Interaction/Rules/RulesetMapInitializer.hpp"

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

struct Context {
    Nebulite::Core::RenderObject& self;
    Nebulite::Core::RenderObject& other;
    Nebulite::Core::GlobalSpace& global;
    // TODO: Parent context?
};

using StaticRulesetFunction = std::function<void(const Context&)>;


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
        std::string topic;
        StaticRulesetFunction function = nullptr;
    };

    StaticRulesetMap() {
        invalidEntry = StaticRuleSetWithMetaData{
            StaticRuleSetWithMetaData::Type::invalid,
            "",
            nullptr
        };
        rulesetMapInit(this);
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
    StaticRuleSetWithMetaData getStaticRulesetByName(std::string const& name) {
        if (container.contains(name)) {
            return container[name];
        }
        return invalidEntry;
    }

    /**
     * @brief Adds a static ruleset function to the map.
     * @param name The name of the static ruleset.
     * @param func Pointer to the static ruleset function, with metadata.
     */
    void bindStaticRuleset(std::string const& name, StaticRuleSetWithMetaData const& func) {
        // Exit program if duplicate
        if (container.contains(name)) {
            throw std::runtime_error("Duplicate static ruleset name: " + name);
        }
        container[name] = func;
    }

private:
    absl::flat_hash_map<std::string, StaticRuleSetWithMetaData> container;
    StaticRuleSetWithMetaData invalidEntry;
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_STATIC_RULESETS_HPP