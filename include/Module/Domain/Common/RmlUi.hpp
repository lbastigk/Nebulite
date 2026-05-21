#ifndef MODULE_DOMAIN_COMMON_RMLUI_HPP
#define MODULE_DOMAIN_COMMON_RMLUI_HPP

// TODO: A module with just a destructor that removes any Renderer RmlUi documents based on the domain id

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Execution/Domain.hpp"
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
class RmlUi final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override {
        return Constants::Event::Success;
    }
    void reinit() override {}

    explicit RmlUi(ConstructorParams const& params) : DomainModule(params) {}

    ~RmlUi() override ;

    RmlUi(RmlUi const&) = delete;
    RmlUi& operator=(RmlUi const&) = delete;
    RmlUi(RmlUi&&) = delete;
    RmlUi& operator=(RmlUi&&) = delete;
};
} // namespace Nebulite::Module::Domain::Common
#endif // MODULE_DOMAIN_COMMON_RMLUI_HPP
