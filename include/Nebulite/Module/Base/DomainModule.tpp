#ifndef NEBULITE_MODULE_BASE_DOMAINMODULE_TPP
#define NEBULITE_MODULE_BASE_DOMAINMODULE_TPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Module/Base/DomainModuleBase.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_MODULE_BASE_DOMAINMODULE_HPP
    #include "Nebulite/Module/Base/DomainModule.hpp"
#endif // NEBULITE_MODULE_BASE_DOMAINMODULE_HPP

//------------------------------------------
namespace Nebulite::Module::Base {

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

} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_DOMAINMODULE_TPP
