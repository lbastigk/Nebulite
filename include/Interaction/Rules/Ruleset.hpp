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

//namespace Nebulite::Core {
//class RenderObject;
//} // namespace Nebulite::Core

namespace Nebulite::Interaction::Execution {
class DomainBase;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction::Rules {
/**
 * @class Ruleset
 * @brief Virtual base class for Ruleset types.
 */
class Ruleset {
public:
    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their RenderObject

    Ruleset() = default;
    ~Ruleset() = default;

    Ruleset(Ruleset const&) = delete;
    Ruleset& operator=(Ruleset const&) = delete;
    Ruleset(Ruleset&&) = delete;
    Ruleset& operator=(Ruleset&&) = delete;

    //------------------------------------------
    // Friend classes
    friend class RulesetCompiler;

    //------------------------------------------
    // Methods: Getters

    /**
     * @brief Gets the id of the ruleset.
     * @return The id of the ruleset, as const reference.
     */
    [[nodiscard]] uint32_t const& getId() const { return id; }

    /**
     * @brief Gets the index of the ruleset in the owning RenderObject's list of entries.
     * @return The index of the ruleset, as const reference.
     */
    [[nodiscard]] uint32_t const& getIndex() const { return index; }

    /**
     * @brief Returns the topic of the ruleset.
     * @return The topic of the ruleset, as const reference.
     */
    [[nodiscard]] std::string const& getTopic() const { return topic; }

    /**
     * @brief Returns the estimated computational cost of the ruleset.
     * @return The estimated computational cost of the ruleset.
     */
    [[nodiscard]] size_t const& getEstimatedCost() const { return estimatedCost; }

    /**
     * @brief Checks whether the ruleset is global.
     * @return True if the ruleset is global, false otherwise.
     */
    [[nodiscard]] bool const& isGlobal() const { return _isGlobal; }

    //------------------------------------------
    // Methods: Workflow

    /**
     * @brief Checks if the ruleset is true in the context of the other render object.
     * @param expr The condition to check.
     * @param otherObj The other domain to use as context 'other'.
     * @return True if the ruleset is true in the context of the other render object, false otherwise.
     */
    virtual bool evaluateCondition(Interaction::Execution::DomainBase const* other);

    /**
     * @brief Checks if the ruleset is true in the context of its own RenderObject as otherObj.
     * @return True if the ruleset is true in the context of its own RenderObject, false otherwise.
     */
    virtual bool evaluateCondition();

    /**
     * @brief Applies the ruleset
     * @param contextOther The render object in the other domain.
     */
    virtual void apply(Interaction::Execution::DomainBase* contextOther);

    /**
     * @brief Applies the ruleset to its own RenderObject as contextOther.
     */
    virtual void apply();

protected:
    /**
     * @brief The id of the object that owns this entry; the `self` domain.
     */
    uint32_t id = 0;

    /**
     * @brief The index of this entry in the list of entries of the owning RenderObject.
     */
    uint32_t index = 0;

    /**
     * @brief Indicates whether the ruleset is global or local.
     *        if true, the Ruleset is global and can be broadcasted to other objects: Same as a nonempty topic
     */
    bool _isGlobal = true;

    /**
     * @brief Pointer to the RenderObject that owns this ruleset; the `self` domain.
     */
    Interaction::Execution::DomainBase* selfPtr = nullptr;

    /**
     * @brief Cost of this entry, estimated during parsing.
     */
    size_t estimatedCost = 0;

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
};

/**
 * @class Nebulite::Interaction::Rules::StaticRuleset
 * @brief Represents a single ruleset entry for static rulesets.
 */
class StaticRuleset : public Ruleset {
public:
    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their RenderObject

    StaticRuleset() = default;
    ~StaticRuleset() = default;

    StaticRuleset(StaticRuleset const&) = delete;
    StaticRuleset& operator=(StaticRuleset const&) = delete;
    StaticRuleset(StaticRuleset&&) = delete;
    StaticRuleset& operator=(StaticRuleset&&) = delete;

    //------------------------------------------
    // Friend classes
    friend class RulesetCompiler;

    //------------------------------------------
    // Methods: Workflow

    /**
     * @brief Checks if the ruleset is true in the context of the other render object.
     * @details For StaticRuleset, this always returns true.
     * @param otherObj The other render object to compare against.
     * @return True if the ruleset is true in the context of the other render object, false otherwise.
     */
    bool evaluateCondition(Interaction::Execution::DomainBase const* other) override {return true;};

    /**
     * @brief Checks if the ruleset is true in the context of its own RenderObject as otherObj.
     * @details For StaticRuleset, this always returns true.
     * @return True if the ruleset is true in the context of its own RenderObject, false otherwise.
     */
    bool evaluateCondition() override {return evaluateCondition(selfPtr);}

    /**
     * @brief Applies the ruleset
     * @param contextOther The render object in the other domain.
     */
    void apply(Interaction::Execution::DomainBase* contextOther) override {
        Nebulite::Interaction::ContextBase contextBase{*selfPtr, *contextOther, Nebulite::global()};
        staticFunction(contextBase);
    };

    /**
     * @brief Applies the ruleset to its own RenderObject as contextOther.
     */
    void apply() override {apply(selfPtr);}

private:
    StaticRulesetFunction staticFunction = nullptr;
};

/**
 * @class Nebulite::Interaction::Rules::JsonRuleset
 * @brief Represents a single ruleset entry for json-defined rulesets.
 */
class JsonRuleset : public Ruleset {
public:
    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their RenderObject

    JsonRuleset() = default;
    ~JsonRuleset() = default;

    JsonRuleset(JsonRuleset const&) = delete;
    JsonRuleset& operator=(JsonRuleset const&) = delete;
    JsonRuleset(JsonRuleset&&) = delete;
    JsonRuleset& operator=(JsonRuleset&&) = delete;

    //------------------------------------------
    // Friend classes
    friend class RulesetCompiler;

    //------------------------------------------
    // Methods: Workflow

    /**
     * @brief Checks if the ruleset is true in the context of the other render object.
     * @param otherObj The other render object to compare against.
     * @return True if the ruleset is true in the context of the other render object, false otherwise.
     */
    bool evaluateCondition(Interaction::Execution::DomainBase const* other) override ;

    /**
     * @brief Checks if the ruleset is true in the context of its own RenderObject as otherObj.
     * @return True if the ruleset is true in the context of its own RenderObject, false otherwise.
     */
    bool evaluateCondition() override {return evaluateCondition(selfPtr);}

    /**
     * @brief Applies the ruleset
     * @param contextOther The render object in the other domain.
     */
    void apply(Interaction::Execution::DomainBase* contextOther) override ;

    /**
     * @brief Applies the ruleset to its own RenderObject as contextOther.
     */
    void apply() override {apply(selfPtr);}

private:
    //------------------------------------------
    // 1.) Fields for basic json-defined rulesets

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
     * @brief The expressions that are evaluated and applied to the corresponding domains.
     *        e.g.: `self.key1 = 0`, `other.key2 *= $( sin({self.key2}) * 2 )`, `global.key3 = 1`
     */
    std::vector<Logic::Assignment> assignments;

    /**
     * @brief Estimates the computational cost of this ruleset based on the number of variable references in its expressions.
     */
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
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_RULESET_HPP