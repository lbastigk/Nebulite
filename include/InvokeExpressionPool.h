/**
 * @file InvokeExpressionPool.h
 * @brief Thread-safe pool of InvokeExpression instances for concurrent evaluation.
 *
 * The InvokeExpressionPool class manages a fixed-size array of pre-parsed
 * InvokeExpression objects. Each instance in the pool is protected by its own mutex,
 * allowing multiple threads to evaluate expressions in parallel without interfering
 * with one another.
 *
 * Usage:
 *  - Call parse() once to compile the expression into all pool entries.
 *  - Call eval() from multiple threads; each call acquires an available instance.
 *  - If no instance is immediately available, eval() will block on a randomly chosen one.
 *
 * Key Features:
 *  - Fixed pool size defined by INVOKE_EXPR_POOL_SIZE macro.
 *  - Per-instance locking to avoid a single global mutex bottleneck.
 *  - Randomized acquisition order to evenly distribute workload.
 *  - Drop-in compatible with InvokeExpression public interface (parse, eval, getFullExpression).
 *
 * Example:
 * @code
 * Nebulite::InvokeExpressionPool pool;
 * pool.parse("x * y + 3", documentCache);
 * auto result = pool.eval(selfJson, otherJson, globalJson);
 * @endcode
 *
 * Thread Safety:
 *  - Internally synchronized with per-instance std::mutex locks.
 *  - Multiple threads may safely call eval() concurrently.
 *
 * @note The pool stores the same expression in each entry; per-call variable updates
 *       should be done via the eval() call, not shared state.
 */

#pragma once
#include <array>
#include <mutex>
#include <random>
#include <string>
#include "InvokeExpression.h"
#include "ThreadSettings.h"

namespace Nebulite {

class InvokeExpressionPool {
public:
    InvokeExpressionPool() {
        inUse.fill(false);
    }

    // Disable copy constructor and assignment
    InvokeExpressionPool(const InvokeExpressionPool&) = delete;
    InvokeExpressionPool& operator=(const InvokeExpressionPool&) = delete;

    // Enable move constructor and assignment
    InvokeExpressionPool(InvokeExpressionPool&& other) noexcept
        : pool(std::move(other.pool))
        , inUse(std::move(other.inUse))
        , fullExpression(std::move(other.fullExpression))
    {
        // locks array cannot be moved, so we just reinitialize it
        // This is safe since mutexes should only be moved when not in use
    }

    InvokeExpressionPool& operator=(InvokeExpressionPool&& other) noexcept {
        if (this != &other) {
            pool = std::move(other.pool);
            inUse = std::move(other.inUse);
            fullExpression = std::move(other.fullExpression);
            // locks array stays as-is (new default-constructed mutexes)
        }
        return *this;
    }

    // Matches InvokeExpression::parse
    void parse(const std::string& expr, Nebulite::DocumentCache& documentCache, Nebulite::JSON* self, Nebulite::JSON* global) {
        fullExpression = expr;
        for (auto& e : pool) {
            e.parse(expr, documentCache, self, global);
        }
    }

    // Matches InvokeExpression::eval
    std::string eval(Nebulite::JSON* current_other)
    {
        thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, INVOKE_EXPR_POOL_SIZE - 1);

        // Pick a random pool entry and lock it
        size_t idx = dist(rng);

        std::lock_guard<std::mutex> guard(locks[idx]);
        return pool[idx].eval(current_other);
    }

    // Matches InvokeExpression::getFullExpression
    std::string getFullExpression() const {
        return fullExpression;
    }

private:
    std::array<InvokeExpression, INVOKE_EXPR_POOL_SIZE> pool;
    std::array<std::mutex, INVOKE_EXPR_POOL_SIZE> locks;
    std::array<bool, INVOKE_EXPR_POOL_SIZE> inUse; // optional now
    std::string fullExpression;
};

} // namespace Nebulite
