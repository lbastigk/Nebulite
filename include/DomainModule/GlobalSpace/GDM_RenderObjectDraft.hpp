/*
RenderObjectDraft extends the Global Space Tree to provide an in-memory RenderObject to manipulate and spawn.

!
*/

/**
 * @file GDM_RenderObjectDraft.hpp
 * 
 * @brief Provides RenderObject creation utilities
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Core/RenderObject.hpp"

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
 * @class Nebulite::DomainModule::GlobalSpace::RenderObjectDraft
 * @brief Utilities for creating and manipulating RenderObjects
 * 
 * Allows for the creation and manipulation of RenderObjects in a draft state.
 * Allowing us to easily create draft object to continously spawn.
 */
class RenderObjectDraft : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
public:
    /**
     * @brief Overridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Parse Renderobject-specific functions on the draft
     * 
     * @param argc The argument count
     * @param argv The argument vector: the arguments for the RenderObject to parse.
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error parse(int argc, char* argv[]);

    /**
     * @brief Spawn the created draft object
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error spawnDraft(int argc, char* argv[]);

    /**
     * @brief Reset the draft (does not reset any spawned ones!)
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error resetDraft(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    RenderObjectDraft(std::string moduleName, Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        // Bind functions
        bindSubtree("draft", "Functions to manipulate and spawn RenderObjects in draft state");
        bindFunction(&RenderObjectDraft::parse,       "draft parse",   "Parse Renderobject-specific functions on the draft");
        bindFunction(&RenderObjectDraft::spawnDraft,  "draft spawn",   "Spawn the created draft object");
        bindFunction(&RenderObjectDraft::resetDraft,  "draft reset",   "Reset the draft object (does not reset any spawned ones!)");
    }

private:
    std::unique_ptr<Nebulite::Core::RenderObject> draft;
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite