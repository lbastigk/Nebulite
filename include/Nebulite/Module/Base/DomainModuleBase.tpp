#ifndef NEBULITE_MODULE_BASE_DOMAINMODULEBASE_TPP
#define NEBULITE_MODULE_BASE_DOMAINMODULEBASE_TPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Utility/FunctionIdentity.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_MODULE_BASE_DOMAINMODULEBASE_HPP
    #include "Nebulite/Module/Base/DomainModuleBase.hpp"
#endif // NEBULITE_MODULE_BASE_DOMAINMODULEBASE_HPP

//------------------------------------------
namespace Nebulite::Module::Base {

// TODO: Consider using all of these directly to funcTree

// Implementation: free/static/function-object overload
template <typename Func, typename FuncTreeType>
void DomainModuleBase::bindFunctionStatic(
    FuncTreeType* tree,
    Func functionPtr,
    std::string_view name,
    std::string_view helpDescription
) {
    // Delegate to FuncTree helper to construct FunctionPtr and bind
    auto fp = FuncTreeType::makeFunctionPtr(functionPtr);
    auto fp_identity = Utility::FunctionIdentity(functionPtr);
    if (helpDescription.ends_with('\n')) {
        tree->bindFunction({fp, fp_identity}, name, helpDescription);
    }
    else {
        std::string const withNewline = std::string(helpDescription) + '\n';
        tree->bindFunction({fp, fp_identity}, name, withNewline);
    }
}

// Implementation: object + member-function-pointer overload
template <typename Obj, typename Func, typename FuncTreeType>
void DomainModuleBase::bindFunctionStatic(
    FuncTreeType* tree,
    Obj* objectPtr,
    Func functionPtr,
    std::string_view name,
    std::string_view helpDescription
) {
    static_assert(std::is_member_function_pointer_v<Func>, "This overload requires a member function pointer.");

    // Delegate to FuncTree helper that binds the member pointer to the object
    auto fp = FuncTreeType::makeFunctionPtr(objectPtr, functionPtr);
    auto fp_identity = Utility::FunctionIdentity(objectPtr, functionPtr);
    if (helpDescription.ends_with('\n')) {
        tree->bindFunction({fp, fp_identity}, name, helpDescription);
    }
    else {
        std::string const withNewline = std::string(helpDescription) + '\n';
        tree->bindFunction({fp, fp_identity}, name, withNewline);
    }
}

// --- New definitions: category-last overloads ---

// Non-static overload: member-function pointer (non-const)
template <typename R, typename C, typename... Ps>
void DomainModuleBase::bindFunction(
    R (C::*functionPtr)(Ps...),
    std::string_view name,
    std::string_view helpDescription
) {
    // forward to static helper, binding 'this' as the object pointer of type C*
    bindFunctionStatic(funcTree.get(), static_cast<C*>(this), functionPtr, name, helpDescription);
}

// Non-static overload: member-function pointer (const)
template <typename R, typename C, typename... Ps>
void DomainModuleBase::bindFunction(
    R (C::*functionPtr)(Ps...) const,
    std::string_view name,
    std::string_view helpDescription
) {
    // forward to static helper, binding 'this' as the object pointer of type C*
    bindFunctionStatic(funcTree.get(), static_cast<const C*>(this), functionPtr, name, helpDescription);
}

// Non-static overload: generic free/static/callable
template <typename Func>
void DomainModuleBase::bindFunction(
    Func functionPtr,
    std::string_view name,
    std::string_view helpDescription
) {
    bindFunctionStatic(funcTree.get(), functionPtr, name, helpDescription);
}
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_DOMAINMODULEBASE_TPP
