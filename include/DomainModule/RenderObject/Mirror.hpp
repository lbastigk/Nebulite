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

    Constants::Error mirror_once(int argc, char** argv);
    static std::string_view constexpr mirror_once_name = "mirror once";
    static std::string_view constexpr mirror_once_desc = "Mirrors the object to the GlobalSpace document once on next update\n"
        "\n"
        "Usage: mirror once\n"
        "\n"
        "Mirroring is only done for one frame.\n"
        "Mirrors are stored in the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    Constants::Error mirror_on(int argc, char** argv);
    static std::string_view constexpr mirror_on_name = "mirror on";
    static std::string_view constexpr mirror_on_desc = "Enables mirroring to the GlobalSpace document\n"
        "\n"
        "Usage: mirror on\n"
        "\n"
        "Constant mirroring is active until turned off with 'mirror off'\n"
        "\n"
        "Mirrors are stored in the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    Constants::Error mirror_off(int argc, char** argv);
    static std::string_view constexpr mirror_off_name = "mirror off";
    static std::string_view constexpr mirror_off_desc = "Disables mirroring to the GlobalSpace document\n"
        "\n"
        "Usage: mirror off\n"
        "\n"
        "Constant mirroring is inactive until turned on again with 'mirror on'\n"
        "\n"
        "Mirrors are stored in the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    Constants::Error mirror_delete(int argc, char** argv);
    static std::string_view constexpr mirror_delete_name = "mirror delete";
    static std::string_view constexpr mirror_delete_desc = "Deletes the GlobalSpace document entry for this RenderObject\n"
        "\n"
        "Usage: mirror delete\n"
        "\n"
        "Mirrors are removed from the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    Constants::Error mirror_fetch(int argc, char** argv);
    static std::string_view constexpr mirror_fetch_name = "mirror fetch";
    static std::string_view constexpr mirror_fetch_desc = "Deserializes the RenderObject from the GlobalSpace document entry\n"
        "\n"
        "Usage: mirror fetch\n"
        "\n"
        "Mirrors are fetched from the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    //------------------------------------------
    // Category name
    static std::string_view constexpr mirror_name = "mirror";
    static std::string_view constexpr mirror_desc = "Mirror utilities for RenderObject to GlobalSpace synchronization";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Mirror) {
        (void)bindCategory(mirror_name, mirror_desc);
        bindFunction(&Mirror::mirror_once, mirror_once_name, mirror_once_desc);
        bindFunction(&Mirror::mirror_on, mirror_on_name, mirror_on_desc);
        bindFunction(&Mirror::mirror_off, mirror_off_name, mirror_off_desc);
        bindFunction(&Mirror::mirror_delete, mirror_delete_name, mirror_delete_desc);
        bindFunction(&Mirror::mirror_fetch, mirror_fetch_name, mirror_fetch_desc);
    }

private:
    /**
     * @brief Indicates whether mirroring is currently enabled.
     * @details If true, Mirror::update will sync the RenderObject to the GlobalSpace document.
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
     * @details If the RenderObject has an invalid ID (<1), mirrorKey won't be set.
     * @return Potential errors that occurred during setup
     */
    Constants::Error setupMirrorKey();
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_MIRROR_HPP
