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

// Standard Library
#include <string>
#include <memory>

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
NEBULITE_DOMAINMODULE(::Nebulite::Core::GlobalSpace, RenderObjectDraft) {
public:
    /**
     * @brief Overwridden update function.
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
    Nebulite::Constants::Error draft_parse(int argc, char* argv[]);
    const std::string draft_parse_name = "draft parse";
    const std::string draft_parse_desc = R"(Parse Renderobject-specific functions on the draft.

    Usage: draft parse <function> [args...]

    Use 'draft parse help' to see available functions.

    Examples:

    draft parse set text.str Hello World
    draft parse set posX 100
    draft parse set posY 200
    )";

    /**
     * @brief Spawn the created draft object
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error draft_spawn(int argc, char* argv[]);
    const std::string draft_spawn_name = "draft spawn";
    const std::string draft_spawn_desc = R"(Spawn the created draft object.

    Usage: draft spawn
    )";

    /**
     * @brief Reset the draft (does not reset any spawned ones!)
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error draft_reset(int argc, char* argv[]);
    const std::string draft_reset_name = "draft reset";
    const std::string draft_reset_desc = R"(Reset the draft object.

    This does not reset any spawned ones!

    Usage: draft reset
    )";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, RenderObjectDraft) {
        // Initialize the draft holder with the domain
        draft.setDomain(domain);
        
        // Bind functions
        bindSubtree("draft", "Functions to manipulate and spawn RenderObjects in draft state");
        bindFunction(&RenderObjectDraft::draft_parse,  draft_parse_name,   draft_parse_desc);
        bindFunction(&RenderObjectDraft::draft_spawn,  draft_spawn_name,   draft_spawn_desc);
        bindFunction(&RenderObjectDraft::draft_reset,  draft_reset_name,   draft_reset_desc);
    }

private:
    /**
     * @class DraftHolder
     * @brief Protector struct for draft RenderObject
     * Ensuring the draft is only initialized when accessed through lazy-init
     */
    class DraftHolder{
    private:
        ::std::unique_ptr<Nebulite::Core::RenderObject> ptr;
        Nebulite::Core::GlobalSpace* domain_ptr;
    public:
        DraftHolder() : ptr(nullptr), domain_ptr(nullptr) {}
        DraftHolder(Nebulite::Core::GlobalSpace* domain) : ptr(nullptr), domain_ptr(domain) {}
        ~DraftHolder() = default;

        void setDomain(Nebulite::Core::GlobalSpace* domain) {
            domain_ptr = domain;
        }

        ::std::unique_ptr<Nebulite::Core::RenderObject> & get() {
            if(!ptr && domain_ptr) {
                ptr = ::std::make_unique<Nebulite::Core::RenderObject>(domain_ptr);
            }
            return ptr;
        }
    };

    DraftHolder draft;
    
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite