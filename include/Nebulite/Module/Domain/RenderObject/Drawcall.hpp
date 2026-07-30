#ifndef NEBULITE_MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP
#define NEBULITE_MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP

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
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {
/**
 * @class Nebulite::Module::Domain::RenderObject::Drawcall
 * @brief Drawcall management DomainModule of the RenderObject Domain.
 * @details Contains RenderObject-specific logging functionality.
 */
class Drawcall final : public Base::DomainModule<Core::RenderObject> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event drawcallParse(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const ;
    static auto constexpr drawcallParseName = "drawcall parse";
    static auto constexpr drawcallParseDesc = "Parses a string command into a given drawcall name\n"
        "\n"
        "Usage: drawcall parse <name> <args...>";

    [[nodiscard]] Constants::Event drawcallList(Interaction::Context const& ctx, Interaction::ContextScope& ctxScope) const ;
    static auto constexpr drawcallListName = "drawcall list";
    static auto constexpr drawcallListDesc = "Lists all drawcall objects";

    // TODO: Add more utils: remove, reinit, etc.

    //------------------------------------------
    // Categories

    static auto constexpr drawcallName = "drawcall";
    static auto constexpr drawcallDesc = "Drawcall utilities";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Drawcall(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(drawcallName, drawcallDesc);
        bindFunction(&Drawcall::drawcallParse, drawcallParseName, drawcallParseDesc);
        bindFunction(&Drawcall::drawcallList, drawcallListName, drawcallListDesc);
    }
};
} // namespace Nebulite::Module::Domain::RenderObject
#endif // NEBULITE_MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP
