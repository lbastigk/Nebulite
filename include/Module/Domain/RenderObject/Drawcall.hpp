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

    [[nodiscard]] Constants::Event onDrawcallParse(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const ;
    static auto constexpr onDrawcallParse_name = "on-drawcall parse";
    static auto constexpr onDrawcallParse_desc = "Parses a string command into a given drawcall name\n"
        "\n"
        "Usage: on-drawcall parse <name> <args...>";

    // TODO: Add more utils: list, remove, reinit, etc.

    //------------------------------------------
    // Categories

    static auto constexpr onDrawcall_name = "on-drawcall";
    static auto constexpr onDrawcall_desc = "Functions for performing actions on specific drawcalls";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Drawcall(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(onDrawcall_name, onDrawcall_desc);
        bindFunction(&Drawcall::onDrawcallParse, onDrawcallParse_name, onDrawcallParse_desc);
    }
};
} // namespace Nebulite::Module::Domain::RenderObject
#endif // MODULE_DOMAIN_RENDEROBJECT_DRAWCALL_HPP
