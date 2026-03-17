#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP

#include "Interaction/Execution/DomainModule.hpp"

// Small utilities

namespace Nebulite::Interaction::Execution {

template <typename DomainType>
DomainModule<DomainType>::DomainModule(
    std::string name,
    DomainType& domainReference,
    std::shared_ptr<FuncTree<Constants::Error, Domain&, Data::JsonScope&>> const& funcTreePtr,
    Data::JsonScope& scope,
    Data::JsonScope const& settings
) : DomainModuleBase(
        funcTreePtr,
        scope,
        settings
    ),
    moduleName(std::move(name)),
    domain(domainReference) {}

template <typename DomainType>
DomainModule<DomainType>::~DomainModule() = default;

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP
