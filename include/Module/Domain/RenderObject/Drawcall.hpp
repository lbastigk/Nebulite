#ifndef MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP
#define MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

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
    static auto constexpr drawcallParse_name = "drawcall parse";
    static auto constexpr drawcallParse_desc = "Parses a string command into a given drawcall name\n"
        "\n"
        "Usage: drawcall parse <name> <args...>";

    [[nodiscard]] Constants::Event drawcallList(Interaction::Context const& ctx, Interaction::ContextScope& ctxScope) const ;
    static auto constexpr drawcallList_name = "drawcall list";
    static auto constexpr drawcallList_desc = "Lists all drawcall objects";

    // TODO: Add more utils: remove, reinit, etc.

    //------------------------------------------
    // Categories

    static auto constexpr drawcall_name = "drawcall";
    static auto constexpr drawcall_desc = "Drawcall utilities";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Drawcall(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(drawcall_name, drawcall_desc);
        bindFunction(&Drawcall::drawcallParse, drawcallParse_name, drawcallParse_desc);
        bindFunction(&Drawcall::drawcallList, drawcallList_name, drawcallList_desc);
    }
};
} // namespace Nebulite::Module::Domain::RenderObject
#endif // MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP
