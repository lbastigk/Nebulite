#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP

#include <type_traits>
#include <utility>
#include "Interaction/Execution/DomainModule.hpp"

// Small utilities

namespace Nebulite::Interaction::Execution {

template <typename DomainType>
DomainModule<DomainType>::DomainModule(
    std::string name,
    DomainType& domainReference,
    std::shared_ptr<FuncTree<Constants::Error>> funcTreePtr,
    Data::JsonScopeBase& scope,
    Data::JsonScopeBase const& settings
) : DomainModuleBase(
        std::move(funcTreePtr),
        scope,
        settings
    ),
    moduleName(std::move(name)),
    domain(domainReference) {}

template <typename DomainType>
DomainModule<DomainType>::~DomainModule() = default;

// Implementation: free/static/function-object overload
template <typename Func, typename FuncTreeType>
void DomainModuleBase::bindFunctionStatic(
    FuncTreeType* tree,
    Func functionPtr,
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    // Delegate to FuncTree helper to construct FunctionPtr and bind
    auto fp = FuncTreeType::makeFunctionPtr(functionPtr);
    tree->bindFunction(fp, name, helpDescription);
}

// Implementation: object + member-function-pointer overload
template <typename Obj, typename Func, typename FuncTreeType>
void DomainModuleBase::bindFunctionStatic(
    FuncTreeType* tree,
    Obj* objectPtr,
    Func functionPtr,
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    static_assert(std::is_member_function_pointer_v<Func>, "This overload requires a member function pointer.");

    // Delegate to FuncTree helper that binds the member pointer to the object
    auto fp = FuncTreeType::makeFunctionPtr(objectPtr, functionPtr);
    tree->bindFunction(fp, name, helpDescription);
}

// --- New definitions: category-last overloads ---

// Non-static overload: member-function pointer (non-const)
template <typename R, typename C, typename... Ps>
void DomainModuleBase::bindFunction(
    R (C::*functionPtr)(Ps...),
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    // forward to static helper, binding 'this' as the object pointer of type C*
    bindFunctionStatic(funcTree.get(), static_cast<C*>(this), functionPtr, name, helpDescription);
}

// Non-static overload: member-function pointer (const)
template <typename R, typename C, typename... Ps>
void DomainModuleBase::bindFunction(
    R (C::*functionPtr)(Ps...) const,
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    // forward to static helper, binding 'this' as the object pointer of type C*
    bindFunctionStatic(funcTree.get(), static_cast<const C*>(this), functionPtr, name, helpDescription);
}

// Non-static overload: generic free/static/callable
template <typename Func>
void DomainModuleBase::bindFunction(
    Func functionPtr,
    std::string_view const& name,
    std::string_view const& helpDescription
) {
    bindFunctionStatic(funcTree.get(), functionPtr, name, helpDescription);
}

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_TPP
