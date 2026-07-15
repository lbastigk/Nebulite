#ifndef NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
#define NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <concepts>
#include <cstddef>
#include <exception>
#include <functional>
#include <stdexcept>
#include <thread>
#include <vector>

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
 */
template <typename T, typename UsageReturn>
class RecursionSecure {
    std::vector<T> resourceStack{};
    std::size_t recursionDepth = 0;
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
    UsageReturn use(PrepareF&& prepare, F&& f) {
        assert(std::this_thread::get_id() == constructionThreadId &&
               "RecursionSecure must be used in the same thread it was constructed in!");

        recursionDepth++;

        if (resourceStack.size() <= recursionDepth - 1) {
            resourceStack.emplace_back();
        }

        assert(resourceStack.size() >= recursionDepth &&
               "Resource stack size should be at least the recursion depth.");

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
};
} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
