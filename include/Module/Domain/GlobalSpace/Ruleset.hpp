/**
 * @file Ruleset.hpp
 * @brief Contains the declaration of the Ruleset DomainModule for the GlobalSpace domain.
 */

#ifndef MODULE_DOMAIN_GLOBALSPACE_RULESET_HPP
#define MODULE_DOMAIN_GLOBALSPACE_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/KeyGroup.hpp"
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::Ruleset
 * @brief DomainModule for managing rulesets within the GlobalSpace.
 */
class Ruleset final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event broadcast(std::span<std::string const> const& args) const ;
    static auto constexpr broadcast_name = "ruleset broadcast";
    static auto constexpr broadcast_desc = "Broadcasts a ruleset to its specified topic.\n"
        "Usage: broadcast <ruleset>\n"
        "\n"
        "- ruleset: The ruleset content to be broadcasted.";

    [[nodiscard]] Constants::Event listen(std::span<std::string const> const& args) const ;
    static auto constexpr listen_name = "ruleset listen";
    static auto constexpr listen_desc = "Listens for rulesets on a specified topic.\n"
        "Usage: listen <topic>\n"
        "\n"
        "- topic: The topic to listen for incoming rulesets.";

    //------------------------------------------
    // Categories

    static auto constexpr ruleset_name = "ruleset";
    static auto constexpr ruleset_desc = "Functions for managing rulesets in the GlobalSpace.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Ruleset(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindCategory(ruleset_name, ruleset_desc);
        bindFunction(&Ruleset::broadcast, broadcast_name, broadcast_desc);
        bindFunction(&Ruleset::listen, listen_name, listen_desc);
    }

    struct Key : Data::KeyGroup<"ruleset."> {
        // No keys for now
    };
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // MODULE_DOMAIN_GLOBALSPACE_RULESET_HPP
