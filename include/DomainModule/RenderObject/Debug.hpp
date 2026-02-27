/**
 * @file Debug.hpp
 * @brief Debug functions for the domain RenderObject.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDEROBJECT_DEBUG_HPP
#define NEBULITE_DOMAINMODULE_RENDEROBJECT_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::Debug
 * @brief Debug management for the RenderObject tree DomainModule.
 * @todo Move texture debugging function to Texture Domain later on, once src/dst rect is properly handled there too.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    static Constants::Error eval(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr eval_name = "eval";
    static auto constexpr eval_desc = "Evaluate an expression and execute the result.\n"
        "Example: eval echo $(1+1)\n"
        "\n"
        "Examples:\n\n"
        "eval echo $(1+1)    outputs:    2.000000\n"
        "eval echo {self.id} outputs this objects id\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Debug) {
        // Some functions like selected-object need eval to resolve variables
        BIND_FUNCTION(&Debug::eval, eval_name, eval_desc);

        // TODO: Since texture/src/dst debugging is no longer relevant here, we should add other debug functions later on
        //       - initialized drawcall count
        //       - uninitialized drawcall count
        //       - drawcall metadata
        //       - etc.
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_DOMAINMODULE_RENDEROBJECT_DEBUG_HPP
