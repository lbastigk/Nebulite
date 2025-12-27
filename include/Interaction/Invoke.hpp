/**
 * @file Invoke.hpp
 * @brief This file contains the declaration of the Invoke class, which is responsible for managing
 *        dynamic object logic in the Nebulite engine.
 */

#ifndef NEBULITE_INTERACTION_INVOKE_HPP
#define NEBULITE_INTERACTION_INVOKE_HPP

//------------------------------------------
// Includes 

// Standard library
#include <deque>
#include <string>
#include <thread>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Data/RulesetPairings.hpp"
#include "Interaction/Logic/ExpressionPool.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JSON;
}

namespace Nebulite::Interaction::Rules {
class Ruleset;
}

namespace Nebulite::Interaction::Logic {
class Assignment;
}

//------------------------------------------
namespace Nebulite::Interaction {
/**
 * @class Nebulite::Interaction::Invoke
 * @brief The Invoke class manages dynamic object logic in Nebulite.
 * @details This class is responsible for handling the invocation of functions and the
 *          communication between different render objects within the Nebulite engine.
 *          Interactions work on a self-other-global / self-global basis.
 *          Rulesets consist of:
 *          - a broadcasting topic for the domain 'other' to listen to
 *          - a logical condition necessary to be true
 *          - a list of expressions to evaluate and their corresponding domains 'self', 'other' and 'global'
 *          - a list of function calls to execute on the domains 'self', 'other' and 'global'
 *          Expressions allow for simple value modifications, whereas function calls can encapsulate more complex behavior.
 *          rulesets are designed to be lightweight and easily modifiable, allowing for rapid iteration and experimentation.
 *          They are encoded in a JSON format for easy manipulation and storage.
 */
class Invoke {
public:
    //------------------------------------------
    // General

    Invoke();

    /**
     * @brief Destructor - stops worker threads.
     */
    ~Invoke();

    //------------------------------------------
    // Send/Listen

    /**
     * @brief Broadcasts a ruleset to other render objects.
     * @details Sends the specified ruleset to all render objects
     *          that are listening for the entry's topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Rules::Ruleset> const& entry);

    /**
     * @brief Listens for rulesets on a specific topic.
     * @details Checks the specified render object against all available
     *          rulesets for the given topic. If an entry's logical condition is
     *          satisfied, it is added to the list of pairs for later evaluation.
     * @param listener The listening domain
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener domain.
     */
    void listen(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId);

    //------------------------------------------
    // Updating

    /**
     * @brief Updates all pairs built from RenderObject broadcasting and listening.
     * @details This function iterates through all pairs of rulesets and their associated
     *          render objects, updating their states based on the rulesets.
     *          Example:
     *          RenderObject1 broadcasts entry on topic1 to manipulate other, if other has mass > 0
     *          RenderObject2 listens on topic1, checks the logical condition and if true, adds the pair to the list for later evaluation.
     *          on update, this list is processed to apply the changes.
     *          Changes happen in domain `self`, `other` and `global`.
     */
    void update();

    //------------------------------------------
    // Standalone Expression Evaluation

    /**
     * @brief Evaluates a standalone expression.
     * @details Returns the result as a string. As this happens inside the invoke class,
     *          it has access to the global document as well as the DocumentCache.
     *          An empty document is used for the `self` and `other` context:
     *          - All variable access outside an expression defaults to an empty string
     *          - All variable access inside an expression defaults to 0.0
     * @param input The expression to evaluate.
     * @return The result of the evaluation.
     */
    [[nodiscard]] std::string evaluateStandaloneExpression(std::string const& input) const;

    /**
     * @brief Evaluates a standalone expression with context from a RenderObject.
     * @param input The expression to evaluate.
     * @param selfAndOther The RenderObject providing context for `self` and `other`.
     * @return The result of the evaluation.
     */
    static std::string evaluateStandaloneExpression(std::string const& input, Core::RenderObject const* selfAndOther);

    /**
     * @brief Evaluates a standalone expression and returns the result as a double.
     * @param input The expression to evaluate.
     * @return The result of the evaluation as a double.
     */
    [[nodiscard]] double evaluateStandaloneExpressionAsDouble(std::string const& input) const;

    /**
     * @brief Evaluates a standalone expression with context from a RenderObject and returns the result as a double.
     * @param input The expression to evaluate.
     * @param selfAndOther The RenderObject providing context for `self` and `other`.
     * @return The result of the evaluation as a double.
     */
    static double evaluateStandaloneExpressionAsDouble(std::string const& input, Core::RenderObject const* selfAndOther);

    /**
     * @brief Evaluates a standalone expression and returns the result as a boolean.
     * @param input The expression to evaluate.
     * @return The result of the evaluation as a boolean.
     */
    [[nodiscard]] bool evaluateStandaloneExpressionAsBool(std::string const& input) const;

    /**
     * @brief Evaluates a standalone expression with context from a RenderObject and returns the result as a boolean.
     * @param input The expression to evaluate.
     * @param selfAndOther The RenderObject providing context for `self` and `other`.
     * @return The result of the evaluation as a boolean.
     */
    static bool evaluateStandaloneExpressionAsBool(std::string const& input, Core::RenderObject const* selfAndOther);

private:
    //------------------------------------------
    // Threading Containers

    std::unique_ptr<Data::BroadCastListenPairs> worker[THREADRUNNER_COUNT];

    //------------------------------------------
    // Threading variables
    /**
     * @brief Flag to signal threads to stop.
     */
    std::atomic<bool> stopFlag;
};
} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_INVOKE_HPP
