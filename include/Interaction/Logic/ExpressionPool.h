/**
 * @file ExpressionPool.h
 * @brief Thread-safe pool of Expression instances for concurrent evaluation.
 *
 * This file defines the `ExpressionPool` class.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <array>
#include <mutex>
#include <random>
#include <string>

// Nebulite
#include "Interaction/Logic/Expression.h"
#include "Constants/ThreadSettings.h"       // Poolsize is defined here

//------------------------------------------
namespace Nebulite {
namespace Interaction {
namespace Logic {
/**
 * @class Nebulite::Interaction::Logic::ExpressionPool
 * @brief A thread-safe pool of Expression instances for concurrent evaluation.
 * Manages a fixed-size array of pre-parsed `Expression` objects. 
 * Each instance in the pool is protected by its own mutex, 
 * allowing multiple threads to evaluate expressions in parallel without interfering with one another.
 * 
 * Usage:
 * 
 *  - Call `parse()` once to compile the expression into all pool entries.
 *
 *  - Call `eval()` from multiple threads; each call acquires an available instance.
 *
 *  - If no instance is immediately available, `eval()` will block on a randomly chosen one.
 *
 * Key Features:
 * 
 *  - Fixed pool size defined by `INVOKE_EXPR_POOL_SIZE` macro defined in `ThreadSettings.h`
 * 
 *  - Per-instance locking to avoid a single global mutex bottleneck.
 * 
 *  - Randomized acquisition order to evenly distribute workload.
 * 
 *  - Drop-in compatible with Expression public interface (`parse`, `eval`, `getFullExpression`).
 *
 * Thread Safety:
 * 
 *  - Internally synchronized with per-instance `std::mutex` locks.
 *
 *  - Multiple threads may safely call `eval()` concurrently.
 *
 *  - The pool stores the same expression in each entry of the pool
 */
class ExpressionPool {
public:
    ExpressionPool() {}

    // Disable copy constructor and assignment
    ExpressionPool(const ExpressionPool&) = delete;
    ExpressionPool& operator=(const ExpressionPool&) = delete;

    // Enable move constructor and assignment
    ExpressionPool(ExpressionPool&& other) noexcept
        : pool(std::move(other.pool))
        , fullExpression(std::move(other.fullExpression))
    {
        // locks array cannot be moved, so we just reinitialize it
        // This is safe since mutexes should only be moved when not in use
    }

    ExpressionPool& operator=(ExpressionPool&& other) noexcept {
        if (this != &other) {
            pool = std::move(other.pool);
            fullExpression = std::move(other.fullExpression);
            // locks array stays as-is (new default-constructed mutexes)
        }
        return *this;
    }

    //------------------------------------------
    // Public Functions

    /**
     * @brief Parses the given expression and populates the pool with pre-parsed instances.
     *
     * Matches Nebulite::Interaction::Logic::Expression::parse, but allows for concurrent evaluation across multiple threads.
     *
     * @param expr The expression to parse.
     * @param documentCache The document cache to use during parsing and evaluation.
     * @param self The JSON object representing the "self" context.
     * @param global The JSON object representing the "global" context.
     */
    void parse(const std::string& expr, Nebulite::Utility::DocumentCache& documentCache, Nebulite::Utility::JSON* self, Nebulite::Utility::JSON* global) {
        fullExpression = expr;
        for (auto& e : pool) {
            e.parse(expr, documentCache, self, global);
        }

        // Store if this expression is returnable as double
        _isReturnableAsDouble = pool[0].isReturnableAsDouble();
    }

    /**
     * @brief Evaluates the expression in the context of the given JSON object acting as "other".
     *
     * Matches `Nebulite::Interaction::Logic::Expression::eval`, but allows for concurrent evaluation across multiple threads.
     *
     * @param current_other The JSON object representing the current context.
     * @return The result of the evaluation as a string.
     */
    std::string eval(Nebulite::Utility::JSON* current_other){
        thread_local std::mt19937 rng(std::random_device{}());
        thread_local std::uniform_int_distribution<size_t> dist(0, INVOKE_EXPR_POOL_SIZE - 1);

        // Pick a random pool entry and lock it
        size_t idx = dist(rng);

        std::lock_guard<std::mutex> guard(locks[idx]);
        return pool[idx].eval(current_other);
    }

    /**
     * @brief Evaluates the expression as a double in the context of the given JSON object acting as "other".
     *
     * Matches `Nebulite::Interaction::Logic::Expression::evalAsDouble`, but allows for concurrent evaluation across multiple threads.
     *
     * @param current_other The JSON object representing the current context.
     * @return The result of the evaluation as a double.
     */
    double evalAsDouble(Nebulite::Utility::JSON* current_other){
        thread_local std::mt19937 rng(std::random_device{}());
        thread_local std::uniform_int_distribution<size_t> dist(0, INVOKE_EXPR_POOL_SIZE - 1);

        // Pick a random pool entry and lock it
        size_t idx = dist(rng);

        std::lock_guard<std::mutex> guard(locks[idx]);
        return pool[idx].evalAsDouble(current_other);
    }

    /**
     * @brief Gets the full expression string.
     *
     * Matches `Nebulite::Interaction::Logic::Expression::getFullExpression`.
     *
     * @return The full expression as a string.
     */
    std::string getFullExpression() const {
        return fullExpression;
    }

    /**
     * @brief Checks if the expression is returnable as a double.
     *
     * Matches `Nebulite::Interaction::Logic::Expression::isReturnableAsDouble`.
     *
     * @return True if the expression is returnable as a double, false otherwise.
     */
    bool isReturnableAsDouble() {
        return _isReturnableAsDouble;
    }

private:
    // Pool of expressions
    std::array<Expression, INVOKE_EXPR_POOL_SIZE> pool;

    // Locks for thread safety
    std::array<std::mutex, INVOKE_EXPR_POOL_SIZE> locks;

    //------------------------------------------
    // The following variables are shared across all pool entries
    // But placed here for easy access, without disturbing the pool.

    // Full expression string that was parsed
    std::string fullExpression;

    // Storing info about the expression's returnability
    bool _isReturnableAsDouble;
};
} // namespace Logic
} // namespace Interaction
} // namespace Nebulite
