#ifndef NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_TPP
#define NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_TPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cassert>
#include <exception>
#include <functional>
#include <stdexcept>
#include <thread>

//------------------------------------------
// Conditional include

#ifndef NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
#include "Nebulite/Utility/Coordination/RecursionSecure.hpp"
#endif // NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP

//------------------------------------------
namespace Nebulite::Utility::Coordination {

template<typename T, typename UsageReturn, std::size_t AllocatedRecursionDepth>
template<VoidFunctionOfT<T> PrepareF, FunctionOfTWithReturn<T, UsageReturn> F>
UsageReturn RecursionSecure<T, UsageReturn, AllocatedRecursionDepth>::use(PrepareF&& prepare, F&& f) requires (!std::is_void_v<UsageReturn>){
    assert(
        std::this_thread::get_id() == constructionThreadId &&
        "RecursionSecure must be used in the same thread it was constructed in! "
        "Did you forget to make the variable thread_local?"
    );
    recursionDepth++;
    if (resourceStack.size() <= recursionDepth - 1) {
        // Fallback to manual allocation
        auto resource = T();
        std::invoke(std::forward<PrepareF>(prepare), resource);
        try {
            auto r = std::invoke(std::forward<F>(f), resource);
            recursionDepth--;
            return r;
        } catch (std::exception& e) {
            recursionDepth--;
            throw std::logic_error(e.what());
        }
    }
    auto& resource = resourceStack[recursionDepth - 1];
    std::invoke(std::forward<PrepareF>(prepare), resource);
    try {
        auto r = std::invoke(std::forward<F>(f), resource);
        recursionDepth--;
        return r;
    } catch (std::exception& e) {
        recursionDepth--;
        throw std::logic_error(e.what());
    }
}

template<typename T, typename UsageReturn, std::size_t AllocatedRecursionDepth>
template<VoidFunctionOfT<T> PrepareF, FunctionOfTWithReturn<T, UsageReturn> F>
void RecursionSecure<T, UsageReturn, AllocatedRecursionDepth>::use(PrepareF&& prepare, F&& f) requires (std::is_void_v<UsageReturn>) {
    assert(
        std::this_thread::get_id() == constructionThreadId &&
        "RecursionSecure must be used in the same thread it was constructed in! "
        "Did you forget to make the variable thread_local?"
    );
    recursionDepth++;
    if (resourceStack.size() <= recursionDepth - 1) {
        // Fallback to manual allocation
        auto resource = T();
        std::invoke(std::forward<PrepareF>(prepare), resource);
        try {
            std::invoke(std::forward<F>(f), resource);
            recursionDepth--;
        } catch (std::exception& e) {
            recursionDepth--;
            throw std::logic_error(e.what());
        }
    }
    else {
        auto& resource = resourceStack[recursionDepth - 1];
        std::invoke(std::forward<PrepareF>(prepare), resource);
        try {
            std::invoke(std::forward<F>(f), resource);
            recursionDepth--;
        } catch (std::exception& e) {
            recursionDepth--;
            throw std::logic_error(e.what());
        }
    }
}

} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_TPP
