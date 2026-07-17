#ifndef NEBULITE_INTERACTION_RULES_RULESET_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Nebulite
#include "Nebulite/Interaction/Logic/Assignment.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction::Rules {
struct Listener;
} // namespace Nebulite::Interaction::Rules

namespace Nebulite::Module::Base {
class RulesetModule;
} // namespace Nebulite::Module::Base

//------------------------------------------
namespace Nebulite::Interaction::Rules {
/**
 * @class Ruleset
 * @brief Virtual base class for Ruleset types.
 */
class Ruleset {
public:
    // TODO: store global scope during entry creation?

    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their Domain

    explicit Ruleset(Execution::Domain& contextSelf) : self(contextSelf) {}
    virtual ~Ruleset() = default;

    // Ruleset is owned by a single Domain, no copy or move semantics

    Ruleset(Ruleset const&) = delete;
    Ruleset& operator=(Ruleset const&) = delete;
    Ruleset(Ruleset&&) = delete;
    Ruleset& operator=(Ruleset&&) = delete;

    //------------------------------------------
    // Friend classes
    friend class Construction::RulesetCompiler;

    //------------------------------------------
    // Methods: Getters

    /**
     * @brief Gets the id of the ruleset.
     * @return The id of the ruleset, as const reference.
     */
    [[nodiscard]] std::size_t getId() const ;

    /**
    * @brief Gets the hashed id of the ruleset.
    * @details This is useful for distribution of rulesets, as domain ids may not be equally distributed.
    * @return The hashed id of the ruleset, as const reference.
    */
    [[nodiscard]] std::size_t getIdHashed() const ;

    /**
     * @brief Gets the index of the ruleset in the owning Domain's list of entries.
     * @return The index of the ruleset, as const reference.
     */
    [[nodiscard]] std::size_t getIndex() const { return index; }

    /**
     * @brief Returns the topic of the ruleset.
     * @return The topic of the ruleset, as const reference.
     */
    [[nodiscard]] std::string const& getTopic() const { return topic; }

    /**
     * @brief Returns the estimated computational cost of the ruleset.
     * @return The estimated computational cost of the ruleset.
     */
    [[nodiscard]] std::size_t getEstimatedCost() const { return estimatedCost; }

    /**
     * @brief Checks whether the ruleset is global.
     * @return True if the ruleset is global, false otherwise.
     */
    [[nodiscard]] bool isGlobal() const { return _isGlobal; }

    //------------------------------------------
    // Methods: Workflow

    /**
     * @brief Checks if the ruleset is true in the context of the other render object.
     * @param other The other context
     * @param global The global context
     * @return True if the ruleset is true in the context of the other render object, false otherwise.
     */
    virtual bool evaluateConditionGlobally(Execution::Domain& other, Execution::Domain& global);

    /**
     * @brief Checks if the ruleset is true with its own Domain as context other.
     * @param global The global context
     * @return True if the ruleset is true in the context of its own Domain, false otherwise.
     */
    virtual bool evaluateConditionLocally(Execution::Domain& global);

    /**
     * @brief Applies the ruleset with a full context given
     */
    virtual void applyContext(Context& context, ContextScope& contextScope);

    /**
     * @brief Applies the ruleset
     * @param listener The listener domain.
     * @param global The global context.
     */
    virtual void applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global);

    /**
     * @brief Applies the ruleset to its own Domain as context other.
     * @param global The global context.
     */
    virtual void applyDomain(Execution::Domain& global);

protected:
    /**
     * @brief The index of this entry in the list of entries of the owning Domain.
     */
    std::size_t index = 0;

    /**
     * @brief Indicates whether the ruleset is global or local.
     * @details if true, the Ruleset is global and can be broadcasted to other objects: Same as a nonempty topic
     */
    bool _isGlobal = true;

    /**
     * @brief Pointer to the Domain that owns this ruleset; the `self` domain.
     */
    Execution::Domain& self;

    /**
     * @brief Cost of this entry, estimated during parsing.
     */
    std::size_t estimatedCost = 0;

    /**
     * @brief The topic of the ruleset, used for routing and filtering in the broadcast-listen-model of the Invoke class.
     * @details e.g. `gravity`, `hitbox`, `collision`. `all` is the default value. Any Domain listening should be subscribed to this topic.
     *          However, we are allowed to remove the topic listen `all` from any object, though it is not recommended.
     *          As an example, say we wish to implement a console feature to quickly remove any object.
     *          We can do so by sending an `ambassador` object that finds all other object at location (x,y) and deletes them.
     *          This object would broadcast its invoke to `all`. Removing any objects subscription to `all` makes this impossible.
     *
     *          Due to the large checks needed for `all`, it should only be used when absolutely necessary.
     */
    std::string topic = "all";
};

/**
 * @class Nebulite::Interaction::Rules::StaticRuleset
 * @brief Represents a single ruleset entry for static rulesets.
 * @todo Add proximity-based specialisation, storing the broadcaster/listener tile position
 *       Only do additional checks if max([abs(x1-x2), abs(y1-y2)]) <= 1
 *       Make tilePos optional, if either 1 or 2 is not set, do the additional checks anyways
 *       We need a proper way to store current tile pos of broadcaster and listener,
 *       making sure the data is always up to date!
 *       Examples for proximity-based rulesets: Collision, Attack damage
 */
class StaticRuleset final : public Ruleset {
public:
    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their Domain

    explicit StaticRuleset(Execution::Domain& contextSelf) : Ruleset(contextSelf) {}
    ~StaticRuleset() override = default;

    StaticRuleset(StaticRuleset const&) = delete;
    StaticRuleset& operator=(StaticRuleset const&) = delete;
    StaticRuleset(StaticRuleset&&) = delete;
    StaticRuleset& operator=(StaticRuleset&&) = delete;

    //------------------------------------------
    // Types

    enum class Type : std::uint8_t {
        Local,
        Global,
        invalid
    };

    using Function = void (*)(void*, Context const&, double**, double**);
    using BaseListFunction = std::function<double**(Execution::Domain const&)>;

    //------------------------------------------
    // Friend classes
    friend class Construction::RulesetCompiler;

    //------------------------------------------
    // Methods: Workflow

    /**
     * @brief Checks if the ruleset is true in the context of the other render object.
     * @details For StaticRuleset, this always returns true.
     * @param other The other context
     * @param global The global context.
     * @return True if the ruleset is true in the context of the other render object, false otherwise.
     */
    bool evaluateConditionGlobally(Execution::Domain& other, Execution::Domain& global) override ;

    /**
     * @brief Checks if the ruleset is true with its own Domain as context other.
     * @details For StaticRuleset, this always returns true.
     * @return True if the ruleset is true in the context of its own Domain, false otherwise.
     */
    bool evaluateConditionLocally(Execution::Domain& global) override ;

    /**
     * @brief Applies the ruleset with a full context given
     */
    void applyContext(Context& context, ContextScope& contextScope) override ;

    /**
     * @brief Applies the ruleset
     * @param listener The listener domain.
     * @param global The global context.
     */
    void applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global) override ;

    /**
     * @brief Applies the ruleset to its own Domain as contextOther.
     * @param global The global context.
     */
    void applyDomain(Execution::Domain& global) override ;

private:
    void* instance = nullptr;

    Function staticFunction = nullptr;

    BaseListFunction baseListFunction;

    /**
     * @brief Ordered list of variables for this ruleset to use as self
     */
    double** slf = nullptr;
};

/**
 * @class Nebulite::Interaction::Rules::JsonRuleset
 * @brief Represents a single ruleset entry for json-defined rulesets.
 */
class JsonRuleset final : public Ruleset {
public:
    //------------------------------------------
    // Make Entry non-copyable and non-movable
    // All entries are local to their Domain

    explicit JsonRuleset(Execution::Domain& contextSelf) : Ruleset(contextSelf) {}
    ~JsonRuleset() override = default;

    JsonRuleset(JsonRuleset const&) = delete;
    JsonRuleset& operator=(JsonRuleset const&) = delete;
    JsonRuleset(JsonRuleset&&) = delete;
    JsonRuleset& operator=(JsonRuleset&&) = delete;

    //------------------------------------------
    // Friend classes
    friend class Construction::RulesetCompiler;

    //------------------------------------------
    // Methods: Workflow

    /**
     * @brief Checks if the ruleset is true in the context of the other render object.
     * @param other The other context.
     * @param global The global context.
     * @return True if the ruleset is true in the context of the other render object, false otherwise.
     */
    bool evaluateConditionGlobally(Execution::Domain& other, Execution::Domain& global) override;

    /**
     * @brief Checks if the ruleset is true in the context of its own Domain as otherObj.
     * @return True if the ruleset is true in the context of its own Domain, false otherwise.
     */
    bool evaluateConditionLocally(Execution::Domain& global) override { return evaluateConditionGlobally(self, global); }

    /**
     * @brief Applies the ruleset with a full context given
     */
    void applyContext(Context& context, ContextScope& contextScope) override ;

    /**
     * @brief Applies the ruleset
     * @param listener The listener domain.
     * @param global The global context.
     */
    void applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global) override;

    /**
     * @brief Applies the ruleset to its own Domain as contextOther.
     * @param global The global context.
     */
    void applyDomain(Execution::Domain& global) override ;

private:
    //------------------------------------------
    // 1.) Fields for basic json-defined rulesets

    /**
     * @brief The Logical Argument that determines when the ruleset is triggered.
     * @details Logical Arguments are evaluated inside the Invoke class with access to `self`, `other`, and `global` variables.
     *          e.g. "{self:posX} > {other:posY}"
     */
    std::unique_ptr<Logic::Expression> logicalArg;

    /**
     * @brief The function calls that to be executed on global domain.
     * @details Vector of function calls, e.g. "echo example"
     */
    std::vector<Logic::Expression> functioncalls_global;

    /**
     * @brief The function calls that to be executed on self domain.
     * @details Vector of function calls, e.g. "mirror on"
     */
    std::vector<Logic::Expression> functioncalls_self;

    /**
     * @brief The function calls that to be executed on other domain.
     * @details Vector of function calls, e.g. "mirror on"
     */
    std::vector<Logic::Expression> functioncalls_other;

    /**
     * @brief The expressions that are evaluated and applied to the corresponding domains.
     * @details e.g.: `self:key1 = 0`, `other:key2 *= $( sin({self:key2}) * 2 )`, `global:key3 = 1`
     */
    std::vector<Logic::Assignment> assignments;

    /**
     * @brief Estimates the computational cost of this ruleset based on the number of variable references in its expressions.
     */
    void estimateComputationalCost() {
        // Count number of $ and { in logicalArg
        std::string const& expr = logicalArg->getFullExpression();
        estimatedCost += static_cast<std::size_t>(std::ranges::count(expr.begin(), expr.end(), '$'));

        // Count number of $ and { in exprs
        for (auto const& assignment : assignments) {
            std::string const& value = assignment.getFullExpression();
            estimatedCost += static_cast<std::size_t>(std::ranges::count(value.begin(), value.end(), '$'));
            estimatedCost += static_cast<std::size_t>(std::ranges::count(value.begin(), value.end(), '{'));
        }
    }
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_RULESET_HPP
