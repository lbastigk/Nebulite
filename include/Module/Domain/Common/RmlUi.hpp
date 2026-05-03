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
class RmlUi final : public Interaction::Execution::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override {
        return Constants::Event::Success;
    }
    void reinit() override {}

    explicit RmlUi(ConstructorParams const& params) : DomainModule(params) {}

    ~RmlUi() override ;
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_RMLUI_HPP
