/**
 * @file Invoke.hpp
 * 
 * This file contains the declaration of the Invoke class, which is responsible for managing
 * dynamic object logic in the Nebulite engine.
 */

#pragma once

//------------------------------------------
// Includes 

// General
#include <string>
#include <vector>
#include <deque>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

// External
#include "tinyexpr.h"

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Utility/DocumentCache.hpp"
#include "Utility/JSON.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/ParsedEntry.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite {
  namespace Core {
    class RenderObject;
  }
}

//------------------------------------------
namespace Nebulite{
namespace Interaction{
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
 * Then we link them with an std::map and each ParsedEntry is either static or typed.
 * Using a flag inside ParsedEntry to determine which one it is.
 * If the Deserializer encounters an entry in the Array of invokes that is a string, it looks it up in the map and links it.
 * If it is not found, perhaps linking to an "Error" static invoke that just prints an error message/returns a Nebulite::Constants::Error.
 * 
 * @todo Improve language clarity: Use consistent terminology for "domains/document/target" throughout the documentation.
 */
class Invoke{
public:
    //------------------------------------------
    // General

    /**
     * @brief Constructs an Invoke object.
     * 
     * @param globalDocPtr Pointer to the global JSON document.
     */
    Invoke(Nebulite::Utility::JSON* globalDocPtr);

    /**
     * @brief Destructor - stops worker threads.
     */
    ~Invoke();

    /**
     * @brief Links the invoke object to a global queue for function calls.
     * 
     * @param queue Reference to the global queue.
     */
    void linkTaskQueue(std::deque<std::string>& queue){tasks = &queue;}

    /**
     * @brief Clears all broadcasted invoke entries and pairs.
     */
    void clear();

    //------------------------------------------
    // Getting

    /**
     * @brief Gets the global JSON document pointer.
     */
    Nebulite::Utility::JSON* getGlobalPointer(){return global;};

    /**
     * @brief Gets the global queue for function calls.
     */
    std::deque<std::string>* getTaskQueue(){return tasks;};
    
    //------------------------------------------
    // Send/Listen

    /**
     * @brief Broadcasts an invoke entry to other render objects.
     * 
     * This function sends the specified invoke entry to all render objects
     * that are listening for the entry's topic.
     * 
     * @param entry The invoke entry to broadcast.
     */
    void broadcast(std::shared_ptr<Nebulite::Interaction::ParsedEntry> entry);

    /**
     * @brief Listens for invoke entries on a specific topic.
     * 
     * This function checks the specified render object against all available
     * invoke entries for the given topic. If an entry's logical condition is
     * satisfied, it is added to the list of pairs for later evaluation.
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
     * @return True if the invoke entry is true in the context of the other render object, false otherwise.
     */
    bool isTrueGlobal(std::shared_ptr<Nebulite::Interaction::ParsedEntry> entry, Nebulite::Core::RenderObject* otherObj);

    /**
     * @brief Checks if the invoke entry is true, without any context from other render objects.
     * 
     * @param entry The invoke entry to check.
     * @return True if the invoke entry is true without any context from other render objects, false otherwise.
     */
    bool isTrueLocal(std::shared_ptr<Nebulite::Interaction::ParsedEntry> entry);


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
    void updateLocal(std::shared_ptr<Nebulite::Interaction::ParsedEntry> entries_self);

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
    void updateValueOfKey(
        Nebulite::Interaction::Logic::Assignment::Operation operation, 
        const std::string& key, 
        const std::string& valStr, 
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
    void updateValueOfKey(
        Nebulite::Interaction::Logic::Assignment::Operation operation, 
        const std::string& key, 
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
    void updateValueOfKey(
        Nebulite::Interaction::Logic::Assignment::Operation operation, 
        const std::string& key, 
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
    std::string evaluateStandaloneExpression(const std::string& input);

    /**
     * @brief Evaluates a standalone expression with context from a RenderObject.
     * 
     * @param input The expression to evaluate.
     * @param selfAndOther The RenderObject providing context for `self` and `other`.
     * @return The result of the evaluation.
     */
    std::string evaluateStandaloneExpression(const std::string& input, Nebulite::Core::RenderObject* selfAndOther);

    /**
     * @brief Gets a pointer to the DocumentCache.
     * 
     * This function provides access to the DocumentCache used by the invoke class,
     * allowing for efficient document management and retrieval.
     * 
     * @return A pointer to the DocumentCache.
     */
    Nebulite::Utility::DocumentCache* getDocumentCache() { return &docCache; }

private:
    //------------------------------------------
    // General Variables

    // Documents
    Nebulite::Utility::DocumentCache docCache;
    Nebulite::Utility::JSON* emptyDoc = new Nebulite::Utility::JSON();  // Linking an empty doc is needed for some functions
    Nebulite::Utility::JSON* global = nullptr;                 // Linkage to global doc

    // pointer to queue
    std::deque<std::string>* tasks = nullptr; 

    /**
     * @brief Mutex lock for tasks and buffers.
     * @todo Turn into a normal mutex, see if that works
     */
    mutable std::recursive_mutex globalTasksLock;

    //------------------------------------------
    // Threading Containers

    /**
     * @struct BroadCastListenPair
     * @brief Structure to hold a broadcast-listen pair.
     */
    struct BroadCastListenPair{
        std::shared_ptr<Nebulite::Interaction::ParsedEntry> entry; // The ParsedEntry that was broadcasted
        Nebulite::Core::RenderObject* Obj_other;                   // The object that listened to the Broadcast
        bool active = true;                                        // If false, this pair is skipped during update
    };

    /**
     * @typedef Ruleset
     * @brief A shared pointer to a ParsedEntry.
     * ParsedEntry is owned by its RenderObject, so we use a shared pointer here to avoid ownership issues.
     */
    using Ruleset = std::shared_ptr<Nebulite::Interaction::ParsedEntry>;

    /**
     * @struct ThreadWork
     * @brief Structure to hold work for each thread runner.
     * 
     * Matches broadcast-listen pairs in a threadsafe and predictive manner, where 
     * each self-id modulo THREADRUNNER_COUNT is assigned to a specific thread runner.
     */
    struct ThreadWork{
      absl::flat_hash_map<
          uint32_t,                      // The ID of self. Each ThreadWorks id here is the same in modulo THREADRUNNER_COUNT. So pairs_threadsafe_batched[1] stores ids 11, 21, 31...
          absl::flat_hash_map<
              uint32_t,                  // The ID of other. Can be any number.
              absl::flat_hash_map<
                  uint32_t,              // The index of the ruleset inside the broadcaster. Provides a unique key to identify the ruleset.
                  BroadCastListenPair    // The actual pair of entry and other object.
              >
          >
      > work;
      mutable std::mutex mutex; // Mutex to protect access to the work structure
    };

    /**
     * @struct BroadCastEntries
     * @brief Structure to hold broadcasted entries for each thread runner.
     * 
     * @todo Instead of using this temporary storage, find a way to directly create pairs during the broadcast() phase.
     * This would eliminate the need for this structure and the associated mutex, making the process more efficient.
     * 
     * Perhaps two ThreadWork that we switch between on each frame? + a map for the broadcast topic inside?
     * [topic][id_self][index_ruleset].listeners[id_other]-> BroadCastListenPair
     * Only annoying part is the index we need for unique identification of the ruleset.
     * 
     * [topic][id_self].active = true/false, active if self has broadcasted this frame
     * [topic][id_self].rulesets[idx_ruleset].ptr holds the ruleset pointer self.rulesets[idx_ruleset]
     * [topic][id_self].rulesets[idx_ruleset].listeners[id_other] = BroadCastListenPair
     * 
     * On broadcast, we set:   [topic][id_self].active = true; and set all [topic][id_self].rulesets[idx_ruleset].ptr = entry;
     * On listen, we populate: [topic][id_self].rulesets[idx_ruleset].listeners[id_other] = {[topic][id_self].rulesets[idx_ruleset].ptr, Obj_other, true};
     * Due to the idx_ruleset, we can only have one listener.
     * 
     * During update, we simple swap between two of these structures, and process all active entries.
     * And set all active flags to false.
     * 
     * This should work, but might take some time to implement and test.
     */
    struct BroadCastEntries{
      absl::flat_hash_map<
          std::string,          // The topic of the broadcasted entry
          std::vector<Ruleset>  // The actual entries broadcasted this frame
      > work;
      mutable std::mutex mutex; // Mutex to protect access to the work structure
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
         * @brief Contains Broadcasted Entries for this frame
         * 
         * On update, all entries from BroadcastEntriesNextFrame are swapped here.
         */
        BroadCastEntries entriesThisFrame[THREADRUNNER_COUNT];

        /**
         * @brief Contains Broadcasted Entries for the next frame
         * 
         * Populated during the broadcast() phase.
         * 
         * On update, all entries from BroadcastEntriesThisFrame are swapped here.
         */
        BroadCastEntries entriesNextFrame[THREADRUNNER_COUNT];

        /**
         * @brief Array of thread work structures for managing broadcast-listen pairs.
         * 
         * On update, all pairs from broadcastListenEntriesNextFrame are swapped here.
         */
        ThreadWork pairings[THREADRUNNER_COUNT]; 
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
    void updatePair(std::shared_ptr<Nebulite::Interaction::ParsedEntry> entries_self, Nebulite::Core::RenderObject* Obj_other);
};
} // namespace Interaction
} // namespace Nebulite
