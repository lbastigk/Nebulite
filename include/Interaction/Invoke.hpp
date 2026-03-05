/**
 * @file Invoke.hpp
 * @brief This file contains the declaration of the Invoke class, which is responsible for managing
 *        dynamic object logic in the Nebulite engine.
 */

#ifndef NEBULITE_INTERACTION_INVOKE_HPP
#define NEBULITE_INTERACTION_INVOKE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Data/BroadcastListenContainer/FlatContainer.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Rules {
class Ruleset;
struct Listener;
} // namespace Nebulite::Interaction::Rules

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
    void broadcast(std::shared_ptr<Rules::Ruleset> const& entry) const ;

    /**
     * @brief Listens for rulesets on a specific topic.
     * @details Checks the specified render object against all available
     *          rulesets for the given topic. If an entry's logical condition is
     *          satisfied, it is added to the list of pairs for later evaluation.
     */
    void listen(std::shared_ptr<Rules::Listener> const& listener);

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

    // FlatContainer is the best candidate by far, about 3x-5x faster than other containers in large benchmarks.
    // Test:
    // Set ContainerType, then run:
    // Scripts/Benchmark/BroadcastListenContainer.sh
    //
    // Static Rulesets:
    // Average total time for small benchmark: 3.261400 s
    // Average frame time for large benchmark: 0.009293 s
    // JSON Rulesets:
    // Average total time for small benchmark: 6.303100 s
    // Average frame time for large benchmark: 0.148972 s
    using ContainerType = Data::BroadcastListenContainer::FlatContainer;

    std::unique_ptr<ContainerType> worker[Constants::ThreadSettings::Maximum::invokeWorkerCount];

    size_t activeWorkerCount = Constants::ThreadSettings::getInvokeWorkerCount();

    //------------------------------------------
    // Threading variables

    /**
     * @brief Flag to signal threads to stop.
     */
    std::atomic<bool> stopFlag;
};
} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_INVOKE_HPP
