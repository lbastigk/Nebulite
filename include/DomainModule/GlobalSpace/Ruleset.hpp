/**
 * @file Ruleset.hpp
 * @brief Contains the declaration of the Ruleset DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBAL_SPACE_RULESET_HPP
#define NEBULITE_DOMAINMODULE_GLOBAL_SPACE_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // namespace Nebulite::Core


//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Ruleset
 * @brief DomainModule for managing rulesets within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Ruleset) {
public:
    [[nodiscard]] Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Error broadcast(std::span<std::string const> const& args) const ;
    static auto constexpr broadcast_name = "ruleset broadcast";
    static auto constexpr broadcast_desc = "Broadcasts a ruleset to its specified topic.\n"
        "Usage: broadcast <ruleset>\n"
        "\n"
        "- ruleset: The ruleset content to be broadcasted.";

    [[nodiscard]] Constants::Error listen(std::span<std::string const> const& args) const ;
    static auto constexpr listen_name = "ruleset listen";
    static auto constexpr listen_desc = "Listens for rulesets on a specified topic.\n"
        "Usage: listen <topic>\n"
        "\n"
        "- topic: The topic to listen for incoming rulesets.";

    //------------------------------------------
    // Category names

    static auto constexpr ruleset_name = "ruleset";
    static auto constexpr ruleset_desc = "Functions for managing rulesets in the GlobalSpace.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Ruleset) {
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
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBAL_SPACE_RULESET_HPP
