#ifndef NEBULITE_MODULE_DOMAIN_COMMON_RMLUI_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_RMLUI_HPP

// TODO: A module with just a destructor that removes any Renderer RmlUi documents based on the domain id

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
NEBULITE_DOMAINMODULE(Nebulite::Interaction::Execution::Domain, RmlUi) {
public:
    [[nodiscard]] Constants::Event updateHook() override {
        return Constants::Event::Success;
    }
    void reinit() override {}

    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Interaction::Execution::Domain, RmlUi) {}

    ~RmlUi() override ;
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_RMLUI_HPP
