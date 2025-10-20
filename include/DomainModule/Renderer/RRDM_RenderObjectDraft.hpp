/**
 * @file RRDM_RenderObjectDraft.hpp
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
        class Renderer; // Forward declaration of domain class Renderer
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::RenderObjectDraft
 * @brief Utilities for creating and manipulating RenderObjects
 * 
 * Allows for the creation and manipulation of RenderObjects in a draft state.
 * Allowing us to easily create draft object to continously spawn.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, RenderObjectDraft) {
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

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
    Nebulite::Constants::Error draft_parse(int argc,  char* argv[]);
    static const std::string draft_parse_name;
    static const std::string draft_parse_desc;

    /**
     * @brief Spawn the created draft object
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error draft_spawn(int argc,  char* argv[]);
    static const std::string draft_spawn_name;
    static const std::string draft_spawn_desc;

    /**
     * @brief Reset the draft (does not reset any spawned ones!)
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error draft_reset(int argc,  char* argv[]);
    static const std::string draft_reset_name;
    static const std::string draft_reset_desc;

    //------------------------------------------
    // Category names
    static const std::string draft_name;
    static const std::string draft_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, RenderObjectDraft) {
        // Initialize the draft holder with the domain
        draft.setDomain(global);
        
        // Bind functions
        bindCategory(draft_name, &draft_desc);
        bindFunction(&RenderObjectDraft::draft_parse,  draft_parse_name,   &draft_parse_desc);
        bindFunction(&RenderObjectDraft::draft_spawn,  draft_spawn_name,   &draft_spawn_desc);
        bindFunction(&RenderObjectDraft::draft_reset,  draft_reset_name,   &draft_reset_desc);
    }

private:
    /**
     * @class DraftHolder
     * @brief Protector struct for draft RenderObject
     * Ensuring the draft is only initialized when accessed through lazy-init
     */
    class DraftHolder{
    private:
        std::unique_ptr<Nebulite::Core::RenderObject> ptr;
        Nebulite::Core::GlobalSpace* domain_ptr;
    public:
        DraftHolder() : ptr(nullptr), domain_ptr(nullptr) {}
        DraftHolder(Nebulite::Core::GlobalSpace* domain) : ptr(nullptr), domain_ptr(domain) {}

        void setDomain(Nebulite::Core::GlobalSpace* domain) {
            domain_ptr = domain;
        }

        std::unique_ptr<Nebulite::Core::RenderObject> & get() {
            if(!ptr && domain_ptr) {
                ptr = std::make_unique<Nebulite::Core::RenderObject>(domain_ptr);
            }
            return ptr;
        }
    };

    /**
     * @brief The draft RenderObject instance
     */
    DraftHolder draft;
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite