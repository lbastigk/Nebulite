#ifndef MODULE_DOMAIN_ENVIRONMENT_SELECTEDOBJECT_HPP
#define MODULE_DOMAIN_ENVIRONMENT_SELECTEDOBJECT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Environment;
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Environment {
/**
 * @class Nebulite::Module::Domain::Environment::SelectedObject
 * @brief Provides functions to select and interact with a selected RenderObject
 */
class SelectedObject final : public Base::DomainModule<Core::Environment> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    [[nodiscard]] Constants::Event selectedObjectUpdate() const ;
    static auto constexpr selectedObjectUpdateName = "selected-object update";
    static auto constexpr selectedObjectUpdateDesc = "Updates the currently selected object.";

    [[nodiscard]] Constants::Event selectedObjectGet(int argc, char const** argv);
    static auto constexpr selectedObjectGet_name = "selected-object get";
    static auto constexpr selectedObjectGet_desc = "Get a renderobject by its index in the Renderer.\n"
        "The index is converted to its corresponding Domain ID and selected as the current RenderObject to interact with for other selected-object commands.\n"
        "\n"
        "Usage: selected-object get <idx>\n";

    [[nodiscard]] Constants::Event selectedObjectParse(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr selectedObjectParse_name = "selected-object parse";
    static auto constexpr selectedObjectParse_desc = "Parse a command on the selected RenderObject.\n"
        "\n"
        "Usage: selected-object parse <command>\n"
        "\n"
        "Use 'selected-object get <id>' to select a RenderObject first.\n"
        "Use 'selected-object parse help' to see available commands for the selected object.\n";

    //------------------------------------------
    // Categories

    static auto constexpr cam_name = "cam";
    static auto constexpr cam_desc = "Renderer Camera Functions";

    static auto constexpr selectedObject_name = "selected-object";
    static auto constexpr selectedObject_desc = "Functions to select and interact with a selected RenderObject";

    static auto constexpr env_name = "env";
    static auto constexpr env_desc = "Environment management functions";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     * @todo Add domainModules for camera and renderobject-selection and move respective functions in there.
     */
    explicit SelectedObject(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(selectedObject_name, selectedObject_desc);
        bindFunction(&SelectedObject::selectedObjectUpdate, selectedObjectUpdateName, selectedObjectUpdateDesc);
        bindFunction(&SelectedObject::selectedObjectGet, selectedObjectGet_name, selectedObjectGet_desc);
        bindFunction(&SelectedObject::selectedObjectParse, selectedObjectParse_name, selectedObjectParse_desc);
    }

private:
    /**
     * @brief Pointer to the currently selected RenderObject
     * @todo Move pointer ownership to env, so that we can unselect it if the object is deleted?
     */
    Core::RenderObject* selectedRenderObject = nullptr;
    Data::JsonScope* selectedRenderObjectData = nullptr;
};
} // namespace Nebulite::Module::Domain::Environment
#endif // MODULE_DOMAIN_ENVIRONMENT_SELECTEDOBJECT_HPP
