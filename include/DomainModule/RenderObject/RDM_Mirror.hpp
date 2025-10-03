/**
 * @file RDM_Mirror.hpp
 * @brief Header file for the Mirror DomainModule of the RenderObject tree.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class RenderObject; // Forward declaration of domain class RenderObject
    }
}

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace RenderObject{
/**
 * @class Nebulite::DomainModule::RenderObject::Mirror
 * @brief Mirror DomainModule of the RenderObject Domain.
 * 
 * Contains RenderObject-specific Mirror functionality, syncing data with the GlobalSpace document.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Mirror) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Mirrors the object to the GlobalSpace document once
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error mirrorOnce(int argc, char* argv[]);

    /**
     * @brief Enables mirroring to the GlobalSpace document
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error mirrorOn(int argc, char* argv[]);

    /**
     * @brief Disables mirroring to the GlobalSpace document
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error mirrorOff(int argc, char* argv[]);

    /**
     * @brief Deletes the GlobalSpace document entry for this RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error mirrorDelete(int argc, char* argv[]);

    /**
     * @brief Deserializes the RenderObject from the GlobalSpace document entry
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error mirrorFetch(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Mirror) {
        bindSubtree("mirror", "Mirror functions for RenderObject");
        bindFunction(&Mirror::mirrorOnce,  "mirror once",      "Mirrors the object to the GlobalSpace document once on next update");
        bindFunction(&Mirror::mirrorOn,    "mirror on",        "Enables mirroring to the GlobalSpace document");
        bindFunction(&Mirror::mirrorOff,   "mirror off",       "Disables mirroring to the GlobalSpace document");
        bindFunction(&Mirror::mirrorDelete,"mirror delete",    "Deletes the GlobalSpace document entry for this RenderObject");
        bindFunction(&Mirror::mirrorFetch, "mirror fetch",     "Deserializes the RenderObject from the GlobalSpace document entry");
    }

private:
    /**
     * @brief Indicates whether mirroring is currently enabled.
     * 
     * If true, Mirror::update will sync the RenderObject to the GlobalSpace document.
     */
    bool mirrorEnabled = false;

    /**
     * @brief Indicates whether mirroring should be done once.
     */
    bool mirrorOnceEnabled = false;

    /**
     * @brief The key used for mirroring in the GlobalSpace document.
     */
    std::string mirrorKey;

    /**
     * @brief Sets up the mirrorKey based on the RenderObject's ID.
     * 
     * If the RenderObject has an invalid ID (<1), mirrorKey wont be set.
     * 
     * @return Potential errors that occured during setup
     */
    Nebulite::Constants::Error setupMirrorKey();
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite