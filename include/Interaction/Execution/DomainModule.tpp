#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP

#include "Interaction/Execution/DomainModule.hpp"

namespace Nebulite::Interaction::Execution {

template <typename DomainType>
template <typename FuncTreeType, typename ReturnType, typename... Args>
void DomainModule<DomainType>::bindFunctionStatic(
    FuncTreeType* tree,
    ReturnType (*functionPtr)(Args...),
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    // Bind the wrapped function to the FuncTree
    tree->bindFunction(functionPtr, name, helpDescription);
}

//------------------------------------------

template <typename DomainType>
template <typename Func>
void DomainModule<DomainType>::bindFunction(
    Func methodPtr,
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    bindFunctionStatic(funcTree.get(), methodPtr, name, helpDescription);
}

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP
