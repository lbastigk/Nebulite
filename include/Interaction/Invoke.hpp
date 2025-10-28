/**
 * @file Invoke.hpp
 * 
 * This file contains the declaration of the Invoke class, which is responsible for managing
 * dynamic object logic in the Nebulite engine.
 */

#ifndef NEBULITE_INTERACTION_INVOKE_HPP
#define NEBULITE_INTERACTION_INVOKE_HPP

//------------------------------------------
// Includes 

// Standard library
#include <string>
#include <deque>
#include <thread>
#include <condition_variable>
#include <atomic>

// External
#include <tinyexpr.h>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Utility/DocumentCache.hpp"
#include "Utility/JSON.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Ruleset.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
  namespace Core{
    class RenderObject;
  }
}

//------------------------------------------
namespace Nebulite::Interaction{
/**
 * @class Nebulite::Interaction::Invoke
 * @brief The Invoke class manages dynamic object logic in Nebulite.
 * 
 * This class is responsible for handling the invocation of functions and the
 * communication between different render objects within the Nebulite engine.
 * 
 * Interactions work on a self-other-global / self-global basis.
 * 
 * Invoke Entries consist of:
 * 
 * - a broadcasting topic for the domain 'other' to listen to
 * 
 * - a logical condition necessary to be true
 * 
 * - a list of expressions to evaluate and their corresponding domains 'self', 'other' and 'global'
 * 
 * - a list of function calls to execute on the domains 'self', 'other' and 'global'
 * 
 * Expressions allow for simple value modifications, whereas function calls can encapsulate more complex behavior.
 * Invoke entries are designed to be lightweight and easily modifiable, allowing for rapid iteration and experimentation.
 * They are encoded in a JSON format for easy manipulation and storage.
 * 
 * @todo Static and typed invokes
 *       Static invokes point to a predefined function in the engine, e.g. `gravity`, `collision`, `hitbox`.
 *       Typed invokes are loaded from a json file
 *       This allows us to either have a fast, hardcoded invoke or a flexible, user-defined one.
 * Examples:
 * ```cpp
 * void StaticInvokeExample(GlobalSpace* global, RenderObject* self, RenderObject* other){
 *     // Full implementation of a static invoke
 *     // Using:
 *     // domain->set<type>(key, value);         for setting values
 *     // domain->get<type>(key, defaultValue);  for getting values
 *     // domain->getDoublePointer(key);         for quick access to a double value, perhaps faster?
 *     // domain->parseStr(commandStr);          for executing a nebulite command
 * }
 * ```
 * Then we link them with an std::map and each Ruleset is either static or typed.
 * Using a flag inside Ruleset to determine which one it is.
 * If the RulesetCompiler encounters an entry in the Array of invokes that is a string, it looks it up in the map and links it.
 * If it is not found, perhaps linking to an "Error" static invoke that just prints an error message/returns a Nebulite::Constants::Error.
 */
class Invoke{
public:
    //------------------------------------------
    // General

    /**
     * @brief Constructs an Invoke object.
     * 
     * @param globalSpace Pointer to the global space.
     */
    explicit Invoke(Nebulite::Core::GlobalSpace* globalSpace);

    /**
     * @brief Destructor - stops worker threads.
     */
    ~Invoke();

    /**
     * @brief Links the invoke object to a global queue for function calls.
     * 
     * @param queue Reference to the global queue.
     */
    void linkTaskQueue(std::deque<std::string>& queue){taskQueue.ptr = &queue;}

    //------------------------------------------
    // Getting

    /**
     * @brief Gets the global JSON document pointer.
     */
    Nebulite::Utility::JSON* getGlobalPointer(){return globalDoc;}

    /**
     * @brief Gets a pointer to the DocumentCache.
     * 
     * This function provides access to the DocumentCache used by the invoke class,
     * allowing for efficient document management and retrieval.
     * 
     * @return A pointer to the DocumentCache.
     */
    Nebulite::Utility::DocumentCache* getDocumentCache(){ return docCache; }
    
    //------------------------------------------
    // Send/Listen

    /**
     * @brief Broadcasts an invoke entry to other render objects.
     * 
     * This function sends the specified invoke entry to all render objects
     * that are listening for the entry's topic.
     * 
     * @param entry The invoke entry to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Nebulite::Interaction::Ruleset> entry);

    /**
     * @brief Listens for invoke entries on a specific topic.
     * 
     * This function checks the specified render object against all available
     * invoke entries for the given topic. If an entry's logical condition is
     * satisfied, it is added to the list of pairs for later evaluation.
     * 
     * WARNING: This function must not be called concurrently with `update()`,
     * as it may lead to race conditions and undefined behavior!
     * 
     * @param obj The render object to check.
     * @param topic The topic to listen for.
     */
    void listen(Nebulite::Core::RenderObject* obj,std::string topic, uint32_t listenerId);

    //------------------------------------------
    // Value checks

    /**
     * @brief Checks if the invoke entry is true in the context of the other render object.
     * 
     * @param entry The invoke entry to check.
     * @param otherObj The other render object to compare against.
     * Make sure entry and otherObj are not the same object!
     * @return True if the invoke entry is true in the context of the other render object, false otherwise.
     */
    bool checkRulesetLogicalCondition(std::shared_ptr<Nebulite::Interaction::Ruleset> entry, Nebulite::Core::RenderObject const* otherObj);

    /**
     * @brief Checks if the invoke entry is true, without any context from other render objects.
     * 
     * @param entry The invoke entry to check.
     * @return True if the invoke entry is true without any context from other render objects, false otherwise.
     */
    bool checkRulesetLogicalCondition(std::shared_ptr<Nebulite::Interaction::Ruleset> entry);


    //------------------------------------------
    // Updating

    /**
     * @brief Updates all pairs built from RenderObject broadcasting and listening.
     * 
     * This function iterates through all pairs of invoke entries and their associated
     * render objects, updating their states based on the Invoke Entries.
     * 
     * Example:
     * 
     * RenderObject1 broadcasts entry on topic1 to manipulate other, if other has mass > 0
     * RenderObject2 listens on topic1, checks the logical condition and if true, adds the pair to the list for later evaluation.
     * on update, this list is processed to apply the changes.
     * Changes happen in domain `self`, `other` and `global`.
     */
    void update();
    
    /**
     * @brief Updates a RenderObject based on its local entries.
     * 
     * This function processes the local invoke entries for the given render object,
     * applying any changes or updates as necessary. No broadcast/listening necessary, as no other objects are involved.
     * Changes happen in domain `self` and `global`.
     */
    void applyRulesets(std::shared_ptr<Nebulite::Interaction::Ruleset> entries_self);

    /**
     * @brief Updates the value of a specific key in the document.
     * 
     * This function applies the specified operation to the given key and value,
     * updating the document accordingly.
     * 
     * @param operation The operation to perform (set, multiply, concat, etc.).
     * @param key The key to update.
     * @param valStr The new value as a string.
     * @param target The JSON document to update.
     */
    void setValueOfKey(
        Nebulite::Interaction::Logic::Assignment::Operation operation, 
        std::string const& key, 
        std::string const& valStr, 
        Nebulite::Utility::JSON* target
    );

    /**
     * @brief Updates the value of a specific key in the document.
     * 
     * This function applies the specified operation to the given key and value,
     * updating the document accordingly.
     * 
     * @param operation The operation to perform (set, multiply, concat, etc.).
     * @param key The key to update.
     * @param valStr The new value as a double.
     * @param target The JSON document to update.
     */
    void setValueOfKey(
        Nebulite::Interaction::Logic::Assignment::Operation operation, 
        std::string const& key, 
        double value, 
        Nebulite::Utility::JSON* target
    );

    /**
     * @brief Updates the value of a specific key in the document.
     * 
     * This function applies the specified operation to the given key and value,
     * updating the document accordingly.
     * 
     * @param operation The operation to perform (set, multiply, concat, etc.).
     * @param key The key to update.
     * @param valStr The new value as a double.
     * @param target The double pointer to update.
     */
    void setValueOfKey(
        Nebulite::Interaction::Logic::Assignment::Operation operation, 
        std::string const& key, 
        double value, 
        double* target
    );

    /**
     * @brief Evaluates a standalone expression.
     * 
     * Returns the result as a string. As this happens inside the invoke class, 
     * it has access to the global document as well as the DocumentCache.
     * 
     * An empty document is used for the `self` and `other` context:
     * 
     * - All variable access outside of an expression defaults to an empty string
     * 
     * - All variable access inside of an expression defaults to 0.0
     * 
     * @param input The expression to evaluate.
     * @return The result of the evaluation.
     */
    std::string evaluateStandaloneExpression(std::string const& input);

    /**
     * @brief Evaluates a standalone expression with context from a RenderObject.
     * 
     * @param input The expression to evaluate.
     * @param selfAndOther The RenderObject providing context for `self` and `other`.
     * @return The result of the evaluation.
     */
    std::string evaluateStandaloneExpression(std::string const& input, Nebulite::Core::RenderObject* selfAndOther);

private:
    //------------------------------------------
    // General Variables

    // Link to globalspace
    Nebulite::Core::GlobalSpace* global = nullptr;

    // Documents
    Nebulite::Utility::DocumentCache* docCache = nullptr;                       // DocumentCache for read-only documents, linked on construction
    Nebulite::Utility::JSON* emptyDoc = new Nebulite::Utility::JSON(global);    // Linking an empty doc is needed for some functions
    Nebulite::Utility::JSON* globalDoc = nullptr;                               // Linkage to global doc, linked on construction

    // Task Queue
    struct TaskQueue {
        std::deque<std::string>* ptr;
        mutable std::mutex mutex;
    } taskQueue;

    //------------------------------------------
    // Threading Containers

    /**
     * @struct BroadCastListenPair
     * @brief Structure to hold a broadcast-listen pair.
     */
    struct BroadCastListenPair{
        std::shared_ptr<Nebulite::Interaction::Ruleset> entry;  // The Ruleset that was broadcasted
        Nebulite::Core::RenderObject* Obj_other;                // The object that listened to the Broadcast
        bool active = true;                                     // If false, this pair is skipped during update
    };

    /**
     * @typedef Ruleset
     * @brief A shared pointer to a Ruleset.
     * Ruleset is owned by its RenderObject, so we use a shared pointer here to avoid ownership issues.
     */
    using Ruleset = std::shared_ptr<Nebulite::Interaction::Ruleset>;

    struct ListenersOnRuleset{
        std::shared_ptr<Nebulite::Interaction::Ruleset> entry;
        absl::flat_hash_map<uint32_t, BroadCastListenPair> listeners; // id_other -> BroadCastListenPair
    };

    struct OnTopicFromId{
        bool active = false;                                        // If false, this is skipped during update
        absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset
    };

    // two of these are needed, and we swap between them on each frame
    using BroadCastListenContainer = absl::flat_hash_map<
        std::string,                       // The topic of the broadcasted entry
        absl::flat_hash_map<
            uint32_t,                      // The ID of self.
            OnTopicFromId                  // The struct containing active flag and rulesets
        >
    >;

    struct MutableBroadCastListenContainer{
        BroadCastListenContainer Container;
        mutable std::mutex mutex; // for read/write access to the container
    };

    /**
     * @struct BroadCasted
     * @brief Structure to hold broadcasted progression.
     * 
     * - Entries this frame
     * 
     * - Entries next frame
     * 
     * - Pairings created from the entries and listeners
     */
    struct BroadCasted{
        /**
         * @brief Container of broadcasted entries for each thread runner.
         * 
         * Populated during the listen phase
         */
        MutableBroadCastListenContainer entriesThisFrame[THREADRUNNER_COUNT];

        /**
         * @brief Container of broadcasted entries for the next frame for each thread runner.
         * 
         * Populated during the broadcast phase and swapped in at the update phase.
         */
        MutableBroadCastListenContainer entriesNextFrame[THREADRUNNER_COUNT];
    } broadcasted;

    //------------------------------------------
    // Threading variables

    /**
     * @brief Array of thread runners for processing broadcast-listen pairs.
     * 
     * Each thread runner processes pairs assigned to it based on the self ID modulo THREADRUNNER_COUNT.
     */
    std::thread threadrunners[THREADRUNNER_COUNT];

    /**
     * @brief Structure to hold threading state.
     */
    struct ThreadState{
        struct IndividualState{
            /**
             * @brief Condition variables for thread synchronization.
             */
            std::condition_variable condition;

            /**
             * @brief Flags to indicate when work is ready for each thread.
             */
            std::atomic<bool> workReady = false;

            /**
             * @brief Flags to indicate when work is finished for each thread.
             */
            std::atomic<bool> workFinished = false;
        }individualState[THREADRUNNER_COUNT];
        /**
         * @brief Flag to signal threads to stop.
         */
        std::atomic<bool> stopFlag;
    } threadState;

    //------------------------------------------
    // Private methods

    /**
     * @brief Updates a build pair of invoke entry with given domain `other`
     * 
     * @param entries_self The invoke entries for the self domain.
     * @param Obj_other The render object in the other domain to update.
     */
    void applyRulesets(std::shared_ptr<Nebulite::Interaction::Ruleset> entries_self, Nebulite::Core::RenderObject* Obj_other);

    /**
     * @brief Applies a single assignment from an invoke entry.
     * 
     * @param entry The invoke entry to apply.
     * @param Obj_other The render object in the other domain to update.
     */
    void applyAssignment(Nebulite::Interaction::Logic::Assignment& assignment, Nebulite::Core::RenderObject const* Obj_self, Nebulite::Core::RenderObject const* Obj_other);

    /**
     * @brief Applies all functioncalls
     */
    void applyFunctionCalls(Nebulite::Interaction::Ruleset& ruleset, Nebulite::Core::RenderObject *Obj_self, Nebulite::Core::RenderObject *Obj_other);
};
} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_INVOKE_HPP
