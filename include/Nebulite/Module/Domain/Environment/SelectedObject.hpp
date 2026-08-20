#ifndef NEBULITE_MODULE_DOMAIN_ENVIRONMENT_SELECTEDOBJECT_HPP
#define NEBULITE_MODULE_DOMAIN_ENVIRONMENT_SELECTEDOBJECT_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Environment;
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

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
    static auto constexpr selectedObjectGetName = "selected-object get";
    static auto constexpr selectedObjectGetDesc = "Get a renderobject by its index in the Renderer.\n"
        "The index is converted to its corresponding Domain ID and selected as the current RenderObject to interact with for other selected-object commands.\n"
        "\n"
        "Usage: selected-object get <idx>\n";

    [[nodiscard]] Constants::Event selectedObjectParse(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr selectedObjectParseName = "selected-object parse";
    static auto constexpr selectedObjectParseDesc = "Parse a command on the selected RenderObject.\n"
        "\n"
        "Usage: selected-object parse <command>\n"
        "\n"
        "Use 'selected-object get <id>' to select a RenderObject first.\n"
        "Use 'selected-object parse help' to see available commands for the selected object.\n";

    //------------------------------------------
    // Categories

    static auto constexpr camName = "cam";
    static auto constexpr camDesc = "Renderer Camera Functions";

    static auto constexpr selectedObjectName = "selected-object";
    static auto constexpr selectedObjectDesc = "Functions to select and interact with a selected RenderObject";

    static auto constexpr envName = "env";
    static auto constexpr envDesc = "Environment management functions";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit SelectedObject(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(selectedObjectName, selectedObjectDesc);
        bindFunction(&SelectedObject::selectedObjectUpdate, selectedObjectUpdateName, selectedObjectUpdateDesc);
        bindFunction(&SelectedObject::selectedObjectGet, selectedObjectGetName, selectedObjectGetDesc);
        bindFunction(&SelectedObject::selectedObjectParse, selectedObjectParseName, selectedObjectParseDesc);
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
#endif // NEBULITE_MODULE_DOMAIN_ENVIRONMENT_SELECTEDOBJECT_HPP
