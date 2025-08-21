/*
RenderObjectDraft extends the Global Space Tree to provide an in-memory RenderObject to manipulate and spawn.

!
*/

/**
 * @file GTE_RenderObjectDraft.h
 * 
 * @brief Provides RenderObject creation utilities
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"
#include "RenderObject.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace 
namespace GlobalSpaceTreeExpansion {

/**
 * @class Nebulite::GlobalSpaceTreeExpansion::RenderObjectDraft
 * @brief Utilities for creating and manipulating RenderObjects
 * 
 * Allows for the creation and manipulation of RenderObjects in a draft state.
 * Allowing us to easily create draft object to continously spawn:
 * ```bash
 * # Creating draft object
 * on-draft <modifier1>
 * on-draft <modifier2>
 * # Spawning object
 * spawn-draft
 * wait 10
 * spawn-draft
 * ```
 * Instead of applying modifierts to each new spawn, we use the in-memory draft.
 * 
 * It also exposes the Renderobject-Internal functions to the gui via a globally accessible help-function:
 * 
 * ```bash
 * ./bin/Nebulite help         # As the RenderObjectTree is not a subTree of GlobalSpaceTree, this will NOT show RenderObject specific help.
 * ./bin/Nebulite draft-help   # However, this will
 * ```
 */
class RenderObjectDraft : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, RenderObjectDraft> {
public:
    using Wrapper<Nebulite::GlobalSpace, RenderObjectDraft>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    /**
     * @brief Prints all available help information for the RenderObjectTree
     * 
     * @param argc The argument count
     * @param argv The argument vector: All additional arguments for the help command
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE draftHelp(int argc, char* argv[]);

    /**
     * @brief Parse Renderobject-specific functions on the draft
     * 
     * @param argc The argument count
     * @param argv The argument vector: the arguments for the RenderObject to parse.
     * See all RenderObjectTree functions for available options.
     * Or use `./bin/Nebulite draft-help` to see all available options.
     * 
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE onDraft(int argc, char* argv[]);

    /**
     * @brief Spawn the created draft object
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE spawnDraft(int argc, char* argv[]);

    /**
     * @brief Reset the draft (does not reset any spawned ones!)
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE resetDraft(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        // Bind functions
        bindFunction(&RenderObjectDraft::draftHelp,   "draft-help",    "Available functions for the RenderObjectDraft");
        bindFunction(&RenderObjectDraft::onDraft,     "on-draft",      "Parse Renderobject-specific functions on the draft");
        bindFunction(&RenderObjectDraft::spawnDraft,  "spawn-draft",   "Spawn the created draft object");
        bindFunction(&RenderObjectDraft::resetDraft,  "reset-draft",   "Reset the draft object (does not reset any spawned ones!)");
    }

private:
    std::unique_ptr<Nebulite::RenderObject> draft;
};
}
}