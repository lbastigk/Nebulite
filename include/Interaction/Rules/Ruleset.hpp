/**
 * @file Ruleset.hpp
 * @brief This file contains the Ruleset struct, representing a single invoke entry of a RenderObject for manipulation.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Logic/ExpressionPool.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Interaction::Rules {
/**
 * @struct Nebulite::Interaction::Rules::Ruleset
 * @brief Represents a single ruleset of a RenderObject for manipulation.
 *        Invokes are parsed into specific structs. Each Renderobject holds its own InvokeEntries.
 *        The struct also contains a pointer to the RenderObject that owns this entry (the broadcaster).
 * @todo Turn into a virtual base class with derived classes for static and json-defined rulesets.
 *       Would need:
 *       - apply()
 *       - isTrue()
 *       - getTopic()
 *       - getSelfId()
 *       - getSelfPtr()
 *       - getCost()
 *       Rest private, make RulesetCompiler a friend class.
 *       Should reduce code overhead and memory usage.
 */
struct Ruleset {
    /**
     * @brief The topic of the ruleset, used for routing and filtering in the broadcast-listen-model of the Invoke class.
     *        e.g. `gravity`, `hitbox`, `collision`. `all` is the default value. Any RenderObject should be subscribed to this topic.
     *        However, we are allowed to remove the topic listen `all` from any object, though it is not recommended.
     *        As an example, say we wish to implement a console feature to quickly remove any object.
     *        We can do so by sending an `ambassador` object that finds all other object at location (x,y) and deletes them.
     *        This object would broadcast its invoke to `all`. Removing any objects subscription to `all` makes this impossible.
     *
     *        Due to the large checks needed for `all`, it should only be used when absolutely necessary.
     */
    std::string topic = "all";

    //------------------------------------------
    // 1.) Fields for basic json-defined rulesets

    /**
     * @brief The id of the object that owns this entry; the `self` domain.
     */
    uint32_t id = 0;

    /**
     * @brief The index of this entry in the list of entries of the owning RenderObject.
     */
    uint32_t index = 0;

    /**
     * @brief The Logical Argument that determines when the ruleset is triggered.
     *        Logical Arguments are evaluated inside the Invoke class with access to `self`, `other`, and `global` variables.
     *        e.g. "{self.posX} > {other.posY}"
     */
    Logic::ExpressionPool logicalArg;

    /**
     * @brief The function calls that to be executed on global domain.
     *        Vector of function calls, e.g. "echo example"
     */
    std::vector<Logic::ExpressionPool> functioncalls_global;

    /**
     * @brief The function calls that to be executed on self domain.
     *        Vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.jsonc"
     */
    std::vector<Logic::ExpressionPool> functioncalls_self;

    /**
     * @brief The function calls that to be executed on other domain.
     *        Vector of function calls, e.g. "add-invoke ./Resources/Invokes/gravity.jsonc"
     */
    std::vector<Logic::ExpressionPool> functioncalls_other;

    /**
     * @brief Indicates whether the ruleset is global or local.
     *        if true, the Ruleset is global and can be broadcasted to other objects: Same as a nonempty topic
     */
    bool isGlobal = true;

    /**
     * @brief Pointer to the RenderObject that owns this ruleset; the `self` domain.
     */
    Core::RenderObject* selfPtr = nullptr;

    // Expressions
    /**
     * @brief The expressions that are evaluated and applied to the corresponding domains.
     *        e.g.: `self.key1 = 0`, `other.key2 *= $( sin({self.key2}) * 2 )`, `global.key3 = 1`
     */
    std::vector<Logic::Assignment> assignments;

    /**
     * @brief Cost of this entry, estimated during parsing.
     */
    size_t estimatedCost = 0;

    void estimateComputationalCost() {
        // Count number of $ and { in logicalArg
        std::string const* expr = logicalArg.getFullExpression();
        estimatedCost += static_cast<size_t>(std::ranges::count(expr->begin(), expr->end(), '$'));

        // Count number of $ and { in exprs
        for (auto const& assignment : assignments) {
            std::string const& value = assignment.getFullExpression();
            estimatedCost += static_cast<size_t>(std::ranges::count(value.begin(), value.end(), '$'));
            estimatedCost += static_cast<size_t>(std::ranges::count(value.begin(), value.end(), '{'));
        }
    }

    //------------------------------------------
    // 2.) Fields for static rulesets

    StaticRulesetFunction staticFunction = nullptr;

    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their RenderObject

    Ruleset() = default;
    ~Ruleset() = default;

    Ruleset(Ruleset const&) = delete;
    Ruleset& operator=(Ruleset const&) = delete;
    Ruleset(Ruleset&&) = delete;
    Ruleset& operator=(Ruleset&&) = delete;
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_RULESET_HPP