/**
 * @file ExpressionPool.hpp
 * @brief Thread-safe pool of Expression instances for concurrent evaluation.
 *
 * This file defines the `ExpressionPool` class.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSION_POOL_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSION_POOL_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <mutex>
#include <string>
#include <thread>

// Nebulite
#include "Constants/ThreadSettings.hpp"       // pool size is defined here
#include "Interaction/Logic/Expression.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {
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
 *  - Fixed pool size defined by `INVOKE_EXPR_POOL_SIZE` macro defined in `ThreadSettings.hpp`
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
    ExpressionPool() = default;

    ~ExpressionPool() = default;

    // Disable copy constructor and assignment
    ExpressionPool(ExpressionPool const&) = delete;
    ExpressionPool& operator=(ExpressionPool const&) = delete;

    // Enable move constructor and assignment
    ExpressionPool(ExpressionPool&& other) noexcept
    : pool(std::move(other.pool)), fullExpression(std::move(other.fullExpression))
    {
        // locks array cannot be moved, so we just reinitialize it
        // This is safe since mutexes should only be moved when not in use
    }

    ExpressionPool& operator=(ExpressionPool&& other) noexcept {
        if (this != &other){
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
     * @param self The JSON object representing the "self" context.
     */
    void parse(std::string const& expr, Utility::JSON* self){
        static_assert(INVOKE_EXPR_POOL_SIZE > 0, "INVOKE_EXPR_POOL_SIZE must be greater than 0");

        fullExpression = expr;

        // Parse each entry in the pool
        for (size_t i = 0; i < INVOKE_EXPR_POOL_SIZE; ++i){
            pool[i].parse(expr, self);
        }

        // Calculate metadata and unique ID from first entry
        #if INVOKE_EXPR_POOL_SIZE > 1
            _isReturnableAsDouble = pool[0].recalculateIsReturnableAsDouble();
            _isAlwaysTrue = pool[0].recalculateIsAlwaysTrue();
        #else
            _isReturnableAsDouble = pool[0].isReturnableAsDouble();
            _isAlwaysTrue = pool[0].isAlwaysTrue();
        #endif
    }

    /**
     * @brief Evaluates the expression in the context of the given JSON object acting as "other".
     *        Matches `Nebulite::Interaction::Logic::Expression::eval`, but allows for concurrent evaluation across multiple threads.
     * @param current_other The JSON object representing the current context.
     * @return The result of the evaluation as a string.
     */
    std::string eval(Utility::JSON* current_other){
        thread_local size_t const idx = std::hash<std::thread::id>{}(std::this_thread::get_id()) % INVOKE_EXPR_POOL_SIZE;
        std::scoped_lock const guard(locks[idx]);
        return pool[idx].eval(current_other);
    }

    /**
     * @brief Evaluates the expression as a double in the context of the given JSON object acting as "other".
     *        Matches `Nebulite::Interaction::Logic::Expression::evalAsDouble`, but allows for concurrent evaluation across multiple threads.
     * @param current_other The JSON object representing the current context.
     * @return The result of the evaluation as a double.
     */
    double evalAsDouble(Utility::JSON* current_other){
        thread_local size_t const idx = std::hash<std::thread::id>{}(std::this_thread::get_id()) % INVOKE_EXPR_POOL_SIZE;
        std::scoped_lock const guard(locks[idx]);
        return pool[idx].evalAsDouble(current_other);
    }

    /**
     * @brief Gets the full expression string.
     *        Matches `Nebulite::Interaction::Logic::Expression::getFullExpression`.
     * @return The full expression as a string.
     */
    [[nodiscard]] std::string const* getFullExpression() const noexcept {
        return &fullExpression;
    }

    /**
     * @brief Gets the full expression string as a string_view.
     */
    [[nodiscard]] std::string_view getFullExpressionStringView() const noexcept { return fullExpression; }

    /**
     * @brief Checks if the expression is returnable as a double.
     *        Matches `Nebulite::Interaction::Logic::Expression::isReturnableAsDouble`.
     * @return True if the expression is returnable as a double, false otherwise.
     */
    [[nodiscard]] bool isReturnableAsDouble() const noexcept {
        return _isReturnableAsDouble;
    }

    /**
     * @brief Checks if the expression is always true (i.e., "1").
     * @return True if the expression is always true, false otherwise.
     */
    [[nodiscard]] bool isAlwaysTrue() const noexcept {
        return _isAlwaysTrue;
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
    bool _isReturnableAsDouble = false;

    // If the expression is just "1", meaning always true
    bool _isAlwaysTrue = false;
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_POOL_HPP
