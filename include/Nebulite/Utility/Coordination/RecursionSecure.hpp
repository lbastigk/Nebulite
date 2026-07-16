#ifndef NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
#define NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <thread>

//------------------------------------------
// Concepts

template<typename F, typename T>
concept VoidFunctionOfT = requires(F f, T& t) {
    { f(t) } noexcept -> std::same_as<void>;
};

template<typename F, typename T, typename R>
concept FunctionOfTWithReturn = requires(F f, T& t) {
    { f(t) } -> std::same_as<R>;
};

//------------------------------------------
namespace Nebulite::Utility::Coordination {
/**
 * @class RecursionSecure
 * @brief Intended to be used as a thread_local variable to allow recursion-safe access to a resource.
 * @details Solves the constant resource allocation issue.
 * @tparam T The type of the resource to use
 * @tparam UsageReturn The return type of the usage function.
 * @tparam AllocatedRecursionDepth The maximum depth without temporary resource allocation
 */
template <typename T, typename UsageReturn, std::size_t AllocatedRecursionDepth = 16>
class RecursionSecure {
    // A vector resize would invalidate the resource, so we use a fixed size array
    std::array<T, AllocatedRecursionDepth> resourceStack{};

    // Current depth
    std::size_t recursionDepth = 0;

    // The thread ID of the thread that constructed this object
    // Used for assertions
    std::thread::id const constructionThreadId = std::this_thread::get_id();

public:
    RecursionSecure() = default;
    ~RecursionSecure() = default;

    RecursionSecure(const RecursionSecure&) = delete;
    RecursionSecure& operator=(const RecursionSecure&) = delete;
    RecursionSecure(RecursionSecure&&) = delete;
    RecursionSecure& operator=(RecursionSecure&&) = delete;

    /**
     * @brief Use the resource in a recursion-safe manner.
     * @details The resource will be cleaned up using the provided cleanup function after the function f is invoked.
     * @tparam PrepareF The type of the function to prepare the resource. Function signature: void(T&)
     * @tparam F The type of the function to use the resource. Function signature: UsageReturn(T&)
     * @param prepare The function to prepare the resource before use.
     * @param f The function to use the resource
     * @return The return value of the function f.
     */
    template<VoidFunctionOfT<T> PrepareF, FunctionOfTWithReturn<T, UsageReturn> F>
    UsageReturn use(PrepareF&& prepare, F&& f) requires (!std::is_void_v<UsageReturn>);

    /**
     * @brief Use the resource in a recursion-safe manner.
     * @details The resource will be cleaned up using the provided cleanup function after the function f is invoked.
     * @tparam PrepareF The type of the function to prepare the resource. Function signature: void(T&)
     * @tparam F The type of the function to use the resource. Function signature: UsageReturn(T&)
     * @param prepare The function to prepare the resource before use.
     * @param f The function to use the resource
     */
    template<VoidFunctionOfT<T> PrepareF, FunctionOfTWithReturn<T, UsageReturn> F>
    void use(PrepareF&& prepare, F&& f) requires (std::is_void_v<UsageReturn>);
};
} // namespace Nebulite::Utility::Coordination
#include "Nebulite/Utility/Coordination/RecursionSecure.tpp" // NOLINT
#endif // NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
