#ifndef NEBULITE_MODULE_DOMAIN_RENDEROBJECT_LOGGING_HPP
#define NEBULITE_MODULE_DOMAIN_RENDEROBJECT_LOGGING_HPP

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
 * @class Nebulite::Module::Domain::RenderObject::Logging
 * @brief Logging DomainModule of the RenderObject Domain.
 * @details Contains RenderObject-specific logging functionality.
 */
class Logging final : public Base::DomainModule<Core::RenderObject> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event logAll(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr logAllName = "log all";
    static auto constexpr logAllDesc = "Logs the entire RenderObject to a file.\n"
        "\n"
        "Usage: log [filename]\n"
        "\n"
        "Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.\n";

    [[nodiscard]] Constants::Event logKey(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) const ;
    static auto constexpr logKeyName = "log key";
    static auto constexpr logKeyDesc = "Logs a specific key's value to a file.\n"
        "\n"
        "Usage: log key <key> [filename]\n"
        "\n"
        "Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.\n";

    //------------------------------------------
    // Categories

    static auto constexpr logName = "log";
    static auto constexpr logDesc = "Logging utilities";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Logging(ConstructorParams const& params) : DomainModule(params) {
        bindCategory(logName, logDesc);
        bindFunction(&Logging::logAll, logAllName, logAllDesc);
        bindFunction(&Logging::logKey, logKeyName, logKeyDesc);
    }
};
} // namespace Nebulite::Module::Domain::RenderObject
#endif // NEBULITE_MODULE_DOMAIN_RENDEROBJECT_LOGGING_HPP
