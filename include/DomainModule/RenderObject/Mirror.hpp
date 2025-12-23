/**
 * @file Mirror.hpp
 * @brief Header file for the Mirror DomainModule of the RenderObject tree.
 */

#ifndef NEBULITE_RODM_MIRROR_HPP
#define NEBULITE_RODM_MIRROR_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::Mirror
 * @brief Mirror DomainModule of the RenderObject Domain.
 * 
 * Contains RenderObject-specific Mirror functionality, syncing data with the GlobalSpace document.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Mirror) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Mirrors the object to the GlobalSpace document once
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error mirror_once(int argc, char** argv);
    static std::string const mirror_once_name;
    static std::string const mirror_once_desc;

    /**
     * @brief Enables mirroring to the GlobalSpace document
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error mirror_on(int argc, char** argv);
    static std::string const mirror_on_name;
    static std::string const mirror_on_desc;

    /**
     * @brief Disables mirroring to the GlobalSpace document
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error mirror_off(int argc, char** argv);
    static std::string const mirror_off_name;
    static std::string const mirror_off_desc;

    /**
     * @brief Deletes the GlobalSpace document entry for this RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error mirror_delete(int argc, char** argv);
    static std::string const mirror_delete_name;
    static std::string const mirror_delete_desc;

    /**
     * @brief Deserializes the RenderObject from the GlobalSpace document entry
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error mirror_fetch(int argc, char** argv);
    static std::string const mirror_fetch_name;
    static std::string const mirror_fetch_desc;

    //------------------------------------------
    // Category name
    static std::string const mirror_name;
    static std::string const mirror_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Mirror) {
        (void)bindCategory(mirror_name, &mirror_desc);
        bindFunction(&Mirror::mirror_once, mirror_once_name, &mirror_once_desc);
        bindFunction(&Mirror::mirror_on, mirror_on_name, &mirror_on_desc);
        bindFunction(&Mirror::mirror_off, mirror_off_name, &mirror_off_desc);
        bindFunction(&Mirror::mirror_delete, mirror_delete_name, &mirror_delete_desc);
        bindFunction(&Mirror::mirror_fetch, mirror_fetch_name, &mirror_fetch_desc);
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
     * If the RenderObject has an invalid ID (<1), mirrorKey won't be set.
     * 
     * @return Potential errors that occurred during setup
     */
    Constants::Error setupMirrorKey();
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_MIRROR_HPP
