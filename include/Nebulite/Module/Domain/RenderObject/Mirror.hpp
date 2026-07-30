#ifndef NEBULITE_MODULE_DOMAIN_RENDEROBJECT_MIRROR_HPP
#define NEBULITE_MODULE_DOMAIN_RENDEROBJECT_MIRROR_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {
/**
 * @class Nebulite::Module::Domain::RenderObject::Mirror
 * @brief Mirror DomainModule of the RenderObject Domain.
 * 
 * Contains RenderObject-specific Mirror functionality, syncing data with the GlobalSpace document.
 */
class Mirror final : public Base::DomainModule<Core::RenderObject> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event mirrorOnce();
    static auto constexpr mirrorOnceName = "mirror once";
    static auto constexpr mirrorOnceDesc = "Mirrors the object to the GlobalSpace document once on next update\n"
        "\n"
        "Usage: mirror once\n"
        "\n"
        "Mirroring is only done for one frame.\n"
        "Mirrors are stored in the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    [[nodiscard]] Constants::Event mirrorOn();
    static auto constexpr mirrorOnName = "mirror on";
    static auto constexpr mirrorOnDesc = "Enables mirroring to the GlobalSpace document\n"
        "\n"
        "Usage: mirror on\n"
        "\n"
        "Constant mirroring is active until turned off with 'mirror off'\n"
        "\n"
        "Mirrors are stored in the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    [[nodiscard]] Constants::Event mirrorOff();
    static auto constexpr mirrorOffName = "mirror off";
    static auto constexpr mirrorOffDesc = "Disables mirroring to the GlobalSpace document\n"
        "\n"
        "Usage: mirror off\n"
        "\n"
        "Constant mirroring is inactive until turned on again with 'mirror on'\n"
        "\n"
        "Mirrors are stored in the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    [[nodiscard]] Constants::Event mirrorDelete() const ;
    static auto constexpr mirrorDeleteName = "mirror delete";
    static auto constexpr mirrorDeleteDesc = "Deletes the GlobalSpace document entry for this RenderObject\n"
        "\n"
        "Usage: mirror delete\n"
        "\n"
        "Mirrors are removed from the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    [[nodiscard]] Constants::Event mirrorFetch() const ;
    static auto constexpr mirrorFetchName = "mirror fetch";
    static auto constexpr mirrorFetchDesc = "Deserializes the RenderObject from the GlobalSpace document entry\n"
        "\n"
        "Usage: mirror fetch\n"
        "\n"
        "Mirrors are fetched from the GlobalSpace document under key \"mirror.renderObject.id<id>\"\n";

    //------------------------------------------
    // Category name
    static auto constexpr mirrorName = "mirror";
    static auto constexpr mirrorDesc = "Mirror utilities for RenderObject to GlobalSpace synchronization";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Mirror(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(mirrorName, mirrorDesc);
        bindFunction(&Mirror::mirrorOnce, mirrorOnceName, mirrorOnceDesc);
        bindFunction(&Mirror::mirrorOn, mirrorOnName, mirrorOnDesc);
        bindFunction(&Mirror::mirrorOff, mirrorOffName, mirrorOffDesc);
        bindFunction(&Mirror::mirrorDelete, mirrorDeleteName, mirrorDeleteDesc);
        bindFunction(&Mirror::mirrorFetch, mirrorFetchName, mirrorFetchDesc);
    }

    struct Key : Data::KeyGroup<""> { // Requires full access to mirror entire object in update routine.
        // No keys for now
    };

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
    [[nodiscard]] Constants::Event setupMirrorKey();
};
} // namespace Nebulite::Module::Domain::RenderObject
#endif // NEBULITE_MODULE_DOMAIN_RENDEROBJECT_MIRROR_HPP
