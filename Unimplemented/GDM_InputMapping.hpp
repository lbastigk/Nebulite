/**
 * @file GDM_InputMapping.hpp
 * @brief Provides input binding utilities for the Nebulite engine.
 *
 * This file contains a GlobalTree DomainModule to handle input bindings.
 * Note that this file is a work in progress!
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::InputMapping
 * @brief DomainModule for mapping inputs to actions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, InputMapping) {
public:
    /**
     * @brief Updates the input bindings.
     *
     * This function is called to update the state of the input bindings,
     * processing any new input events and updating the binding states.
     * 
     * 1.) Process keyboard input from SDL Renderer
     * 2.) Turn into cross-platform naming
     * 3.) Use keymapping to write state into globalspace
     * 4.) Write deltas into globalspace
     * 5.) Stores last pressed delta, which might become handy if we need to look up the last input state
     *
     * Should use a TimeKeeper so it only updates inputs every n milliseconds
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Reads input mappings from a json file
     * 
     * Uses inputs.jsonc if no filename is provided
     * 
     * Mapping is of the form: `mappings<action:keyAssociations>`
     * 
     * @param argc The argument count
     * @param argv The argument vector: [filename]
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error readMappingsFromFile(int argc, char* argv[]);

    // Useful if we wish to update mappings ingame
    /**
     * @brief Update mappings
     * 
     * @param argc The argument count
     * @param argv The argument vector: <action> <slot> <key> <type>
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error updateInputMapping(int argc, char* argv[]);

    /**
     * @brief Writes the current input mappings to a file
     * 
     * @param argc The argument count
     * @param argv The argument vector: [filename]
     * 
     * Uses inputs.jsonc if no filename is provided
     * 
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error writeMappingsToFile(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, InputMapping){
        // Not implemented yet
        /*
        bindSubtree("input-mapping", "Functions to manage input mappings");
        bindFunction(&InputMapping::readMappingsFromFile,   "input-mapping read-from-file",    "Reads Input Mapping from inputs.jsonc file: [filename]");
        bindFunction(&InputMapping::updateInputMapping,     "input-mapping map",               "Maps one input mapping: <action> <slot> <key> <type>");
        bindFunction(&InputMapping::writeMappingsToFile,    "input-mapping write-to-file",     "Writes Input Mapping to inputs.jsonc file: [filename]");
        */
    }

private:
    /**
     * @brief Represents a key association for input mapping.
     * 
     * The struct represents the Association between a key and its input type.
     */
    struct association{
        std::string key; // e.g. "space"
        enum class type {
            empty,
            current,
            onPress,
            onRelease
        } type;
    };

    /**
     * @brief Represents a mapping entry for input actions.
     * 
     * Any input action can be associated with up to three keys.
     */
    struct mapEntry{
        association slot_1{"", association::type::empty}; // First key associated with the action
        association slot_2{"", association::type::empty}; // Second key associated with the action
        association slot_3{"", association::type::empty}; // Third key associated with the action
    };

    /**
     * @brief Maps input actions to their associated keys.
     */
    absl::flat_hash_map<std::string, mapEntry> mappings;

    /**
     * @todo Implement input mapping association:
     * 
     * Example:
     * 
     * ```cpp
     * 
     *  for (const auto& [action, entry] : mappings) {
     *      // Process each mapping
     *      int current = 0;
     *
     *      for(const auto& association : {entry.slot_1, entry.slot_2, entry.slot_3}) {
     *          switch (association.type) {
     *              case association::type::current:
     *                  current +=     global.get<int>("<locationForCurrent>." + association.key);
     *                  break;
     *              case association::type::onPress:
     *                  current += abs(global.get<int>("<locationForDelta>." + association.key)) == 1;
     *                  break;
     *              case association::type::onRelease:
     *                  current +=     global.get<int>("<locationForDelta>." + association.key) == -1;
     *                  break;
     *              case association::type::empty:
     *                  break;
     *          }
     *      }
     *
     *      // Now we write the state into our mapping location
     *      global.set<int>("<locationForAction>." + action, current);
     * }
     *
     * ```
     */
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite
