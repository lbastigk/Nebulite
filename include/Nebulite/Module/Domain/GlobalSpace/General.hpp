#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_GENERAL_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::General
 * @brief DomainModule for general-purpose functions within the GlobalSpace.
 */
class General final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event exit() const ;
    static auto constexpr exitName = "exit";
    static auto constexpr exitDesc = "Exits the entire program.\n"
        "\n"
        "Usage: exit\n"
        "\n"
        "Closes the program\n"
        "Any queued tasks will be discarded.\n";

    //------------------------------------------
    // Categories

    // None, general functions should be in the root of the GlobalSpace
    // Otherwise they should not be considered general-purpose

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit General(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&General::exit, exitName, exitDesc);
    }
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_GENERAL_HPP
