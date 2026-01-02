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
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Rules {
class Ruleset;
} // namespace Nebulite::Interaction::Rules

namespace Nebulite::Interaction::Logic {
class Assignment;
} // namespace Nebulite::Interaction::Logic

//------------------------------------------
namespace Nebulite::Interaction {
/**
 * @class Nebulite::Interaction::Invoke
 * @brief The Invoke class manages dynamic object logic in Nebulite.
 * @details This class is responsible for handling the invocation of functions and the
 *          communication between different render objects within the Nebulite engine.
 *          Interactions work on a self-other-global / self-global basis.
 * @details JSON Rulesets consist of:
 *          - a broadcasting topic for the domain 'other' to listen to
 *          - a logical condition necessary to be true
 *          - a list of expressions to evaluate and their corresponding domains 'self', 'other' and 'global'
 *          - a list of function calls to execute on the domains 'self', 'other' and 'global'
 * @details Expressions allow for simple value modifications, whereas function calls can encapsulate more complex behavior.
 *          rulesets are designed to be lightweight and easily modifiable, allowing for rapid iteration and experimentation.
 *          They are encoded in a JSON format for easy manipulation and storage.
 * @details Static rulesets are direct c++ code being executed on the domains and offer faster performance for critical interactions.
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
    void listen(Interaction::Execution::DomainBase& listener, std::string const& topic, uint32_t const& listenerId);

    //------------------------------------------
    // Updating

    /**
     * @brief Updates all pairs built from RenderObject broadcasting and listening.
     * @details This function iterates through all pairs of rulesets and their associated
     *          render objects, updating their states based on the rulesets.
     * @details Example:
     *          - RenderObject1 broadcasts entry on topic1 to manipulate other, if other has mass > 0
     *          - RenderObject2 listens on topic1, checks the logical condition and if true, adds the pair to the list for later evaluation.
     *          - on update, this list is processed to apply the changes.
     * @details Changes happen in domain `self`, `other` and `global`.
     */
    void update();

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
