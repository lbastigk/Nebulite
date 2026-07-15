#ifndef NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
#define NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

//------------------------------------------
// Concepts

template<typename T, typename F>
concept VoidFunctionOfT =
    requires(F f, T& t) {
    { f(t) } noexcept -> std::same_as<void>;
};

//------------------------------------------
namespace Nebulite::Utility::Coordination {
/**
 * @class RecursionSecure
 * @brief Intended to be used as a thread_local variable to allow recursion-safe access to a resource.
 * @details Solves the constant resource allocation issue.
 * @tparam T The type of the resource to use
 * @tparam CleanupF The type of the cleanup function. Function signature: void(T&)
 */
template <typename T, VoidFunctionOfT<T> CleanupF>
class RecursionSecure {
    std::vector<T> resourceStack{};
    std::size_t recursionDepth = 0;
    std::thread::id const constructionThreadId = std::this_thread::get_id();
    CleanupF cleanup;

    /**
     * @brief Guard class to ensure cleanup is called when the resource is no longer needed.
     */
    struct CleanupGuard {
        RecursionSecure& self;
        T& resource;
        ~CleanupGuard() noexcept {
            std::invoke(self.cleanup, resource);
            --self.recursionDepth;
        }

        CleanupGuard(RecursionSecure& self, T& resource) : self(self), resource(resource) {}
        CleanupGuard(const CleanupGuard&) = delete;
        CleanupGuard& operator=(const CleanupGuard&) = delete;
        CleanupGuard(CleanupGuard&&) = delete;
        CleanupGuard& operator=(CleanupGuard&&) = delete;
    };

public:
    RecursionSecure(CleanupF cleanupFunction) : cleanup(cleanupFunction) {}
    ~RecursionSecure() = default;

    RecursionSecure(const RecursionSecure&) = delete;
    RecursionSecure& operator=(const RecursionSecure&) = delete;
    RecursionSecure(RecursionSecure&&) = delete;
    RecursionSecure& operator=(RecursionSecure&&) = delete;

    /**
     * @brief Use the resource in a recursion-safe manner.
     * @brief The resource will be cleaned up using the provided cleanup function after the function f is invoked.
     * @tparam F The type of the function to use the resource. Function signature: void(T&)
     * @param f The function to use the resource
     */
    template<VoidFunctionOfT<T> F>
    void use(F&& f) {
        assert(std::this_thread::get_id() == constructionThreadId && "RecursionSecure must be used in the same thread it was constructed in! Make sure it's a thread_local variable.");
        recursionDepth++;
        if (resourceStack.size() <= recursionDepth - 1) {
            resourceStack.emplace_back();
        }
        assert(resourceStack.size() >= recursionDepth && "Resource stack size should be at least the recursion depth.");
        auto& resource = resourceStack[recursionDepth - 1];
        CleanupGuard guard{*this, resource};
        std::invoke(std::forward<F>(f), resource);
    }
};
} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_RECURSIONSECURE_HPP
