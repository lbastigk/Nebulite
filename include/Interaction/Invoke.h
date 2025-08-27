/**
 * @file Invoke.h
 * 
 * This file contains the declaration of the Invoke class, which is responsible for managing
 * dynamic object logic in the Nebulite engine.
 */

#pragma once

// General Includes
#include <string>
#include <vector>
#include <deque>
#include <shared_mutex>
#include <thread>

#include "tinyexpr.h"

// Local
#include "Constants/ThreadSettings.h"

#include "Utility/JSON.h"
#include "Interaction/Logic/Assignment.h"
#include "Interaction/Logic/ParsedEntry.h"
#include "Utility/DocumentCache.h"

//-------------------------------------------
// Forward declarations
namespace Nebulite {
  namespace Core{
    class RenderObject;
  }
}

//-------------------------------------------
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
 * @todo Improve language clarity: Use consistent terminology for "domains/document/target" throughout the documentation.
 */
class Invoke{
public:
    
    // Function to convert a JSON doc of Renderobject into a vector of InvokeEntry


    //--------------------------------------------
    // General

    /**
     * @brief Constructs an Invoke object.
     * 
     * @param globalDocPtr Pointer to the global JSON document.
     */
    Invoke(Nebulite::Utility::JSON* globalDocPtr);

    /**
     * @brief Links the invoke object to a global queue for function calls.
     * 
     * @param queue Reference to the global queue.
     */
    void linkQueue(std::deque<std::string>& queue){tasks = &queue;}

    /**
     * @brief Clears all broadcasted invoke entries and pairs.
     */
    void clear();

    //--------------------------------------------
    // Getting

    /**
     * @brief Gets the global JSON document pointer.
     */
    Nebulite::Utility::JSON* getGlobalPointer(){return global;};

    /**
     * @brief Gets the global queue for function calls.
     */
    std::deque<std::string>* getQueue(){return tasks;};
    
    //--------------------------------------------
    // Send/Listen

    /**
     * @brief Broadcasts an invoke entry to other render objects.
     * 
     * This function sends the specified invoke entry to all render objects
     * that are listening for the entry's topic.
     * 
     * @param entry The invoke entry to broadcast.
     */
    void broadcast(std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry> entry);

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
    void listen(Nebulite::Core::RenderObject* obj,std::string topic);

    //--------------------------------------------
    // Value checks

    /**
     * @brief Checks if the invoke entry is true in the context of the other render object.
     * 
     * @param entry The invoke entry to check.
     * @param otherObj The other render object to compare against.
     * @return True if the invoke entry is true in the context of the other render object, false otherwise.
     */
    bool isTrueGlobal(std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry> entry, Nebulite::Core::RenderObject* otherObj);

    /**
     * @brief Checks if the invoke entry is true, without any context from other render objects.
     * 
     * @param entry The invoke entry to check.
     * @return True if the invoke entry is true without any context from other render objects, false otherwise.
     */
    bool isTrueLocal(std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry> entry);


    //--------------------------------------------
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
    void updateLocal(std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry> entries_self);

    /**
     * @brief Updates the value of a specific key in the document.
     * 
     * This function applies the specified operation to the given key and value,
     * updating the document accordingly.
     * 
     * @param operation The operation to perform (set, multiply, concat, etc.).
     * @param key The key to update.
     * @param valStr The new value as a string.
     * @param doc The JSON document to update.
     */
    void updateValueOfKey(
      Nebulite::Interaction::Logic::Assignment::Operation operation, 
      const std::string& key, 
      const std::string& valStr, 
      Nebulite::Utility::JSON* doc
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
     * @param doc The JSON document to update.
     */
    void updateValueOfKey(
      Nebulite::Interaction::Logic::Assignment::Operation operation, 
      const std::string& key, 
      double value, 
      Nebulite::Utility::JSON* doc
    );

    /**
     * @brief Evaluates a standalone expression.
     * 
     * This function takes a standalone expression as input and evaluates it,
     * returning the result as a string. As this happens inside the invoke class, 
     * it has access to the global document as well as the DocumentCache.
     * 
     * @param input The expression to evaluate.
     * @return The result of the evaluation.
     */
    std::string evaluateStandaloneExpression(const std::string& input);

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
    //----------------------------------------------------------------
    // General Variables

    // Documents
    Nebulite::Utility::DocumentCache docCache;
    Nebulite::Utility::JSON* emptyDoc = new Nebulite::Utility::JSON();  // Linking an empty doc is needed for some functions
    Nebulite::Utility::JSON* global = nullptr;                 // Linkage to global doc

    // pointer to queue
    std::deque<std::string>* tasks = nullptr; 

    // Mutex lock for tasks and buffers
    mutable std::recursive_mutex tasks_lock;
    std::mutex entries_global_next_Mutex;
    std::mutex entries_global_Mutex;
    std::mutex pairsMutex;

    //----------------------------------------------------------------
    // Hashmaps and vectors

    // Current and next commands
    absl::flat_hash_map<
      std::string, 
      std::vector<
        std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>
      >
    > entries_global;

    absl::flat_hash_map<
      std::string, 
      std::vector<
        std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>
      >
    > entries_global_next; 

    // All pairs of last listen
    std::vector<
      std::vector<
        std::pair<
          std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry>,
          Nebulite::Core::RenderObject*
        >
      >
    > pairs_threadsafe;

    
    //----------------------------------------------------------------
    // Private functions

    /**
     * @brief Updates a build pair of invoke entry with given domain `other`
     * 
     * @param entries_self The invoke entries for the self domain.
     * @param Obj_other The render object in the other domain to update.
     */
    void updatePair(std::shared_ptr<Nebulite::Interaction::Logic::ParsedEntry> entries_self, Nebulite::Core::RenderObject* Obj_other);
};
} // namespace Interaction
} // namespace Nebulite
