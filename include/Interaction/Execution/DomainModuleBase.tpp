#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_BASE_TPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_BASE_TPP

#include "Interaction/Execution/DomainModuleBase.hpp"

// TODO: Consider passing all of these directly to funcTree

namespace Nebulite::Interaction::Execution {
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
    auto fp_identity = typename FuncTreeType::FunctionIdentity(functionPtr);
    tree->bindFunction({fp, fp_identity}, name, helpDescription);
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
    auto fp_identity = typename FuncTreeType::FunctionIdentity(objectPtr, functionPtr);
    tree->bindFunction({fp, fp_identity}, name, helpDescription);
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
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINMODULE_BASE_TPP
