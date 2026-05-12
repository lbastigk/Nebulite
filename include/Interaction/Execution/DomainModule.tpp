#ifndef INTERACTION_EXECUTION_DOMAINMODULE_TPP
#define INTERACTION_EXECUTION_DOMAINMODULE_TPP

namespace Nebulite::Interaction::Execution {

template <typename DomainType>
DomainModule<DomainType>::DomainModule(ConstructorParams const& params)
    : DomainModuleBase(
        params.funcTreePtr,
        params.scope,
        params.settings
    ),
    moduleName(std::move(params.name)),
    domain(params.domainReference) {}

template <typename DomainType>
DomainModule<DomainType>::~DomainModule() = default;

} // namespace Nebulite::Interaction::Execution
#endif // INTERACTION_EXECUTION_DOMAINMODULE_TPP
