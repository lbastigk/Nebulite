/**
 * @file ExpressionPool.hpp
 * @brief Defines the ExpressionPool class for managing a pool of Expression instances.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSION_POOL_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSION_POOL_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <string>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Constants/ThreadSettings.hpp"       // pool size is defined here
#include "Interaction/Logic/Expression.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @class Nebulite::Interaction::Logic::ExpressionPool
 * @brief A thread-safe pool of Expression instances for concurrent evaluation.
 * @details Manages a fixed-size array of pre-parsed `Expression` objects.
 *          Each instance in the pool is protected by its own mutex,
 *          allowing multiple threads to evaluate expressions in parallel without interfering with one another.
 *          \n
 *          \n
 *          # Usage
 *          - Call `parse()` once to compile the expression into all pool entries.
 *          - Call `eval()` from multiple threads; each call acquires an available instance.
 *          - If no instance is immediately available, `eval()` will block on a randomly chosen one
 *          .
 *          # Key Features
 *          - Fixed pool size defined by `INVOKE_EXPR_POOL_SIZE` macro defined in `ThreadSettings.hpp`
 *          - Per-instance locking to avoid a single global mutex bottleneck.
 *          - Drop-in compatible with Expression public interface (`parse`, `eval`, `getFullExpression`).
 */
class ExpressionPool {
public:
    explicit ExpressionPool(std::string const& expr) {
        parse(expr);
    }

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

    ExpressionPool& operator=(ExpressionPool&& other) = delete;

    //------------------------------------------
    // Public Functions

    /**
     * @brief Evaluates the expression in the context of the given JSON object acting as "other".
     *        Matches `Nebulite::Interaction::Logic::Expression::eval`, but allows for concurrent evaluation across multiple threads.
     * @return The result of the evaluation as a string.
     */
    [[nodiscard]] std::string eval(Context const& context) const {
        return pool[threadIndex()]->eval(context);
    }
    [[nodiscard]] std::string eval(ContextScope const& context) const {
        return pool[threadIndex()]->eval(context);
    }

    /**
     * @brief Evaluates the expression as a double in the context of the given JSON object acting as "other".
     *        Matches `Nebulite::Interaction::Logic::Expression::evalAsDouble`.
     * @return The result of the evaluation as a double.
     */
    [[nodiscard]] double evalAsDouble(Context const& context) const {
        return pool[threadIndex()]->evalAsDouble(context);
    }
    [[nodiscard]] double evalAsDouble(ContextScope const& context) const {
        return pool[threadIndex()]->evalAsDouble(context);
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
    /**
     * @brief Parses the given expression and populates the pool with pre-parsed instances.
     *        Matches Nebulite::Interaction::Logic::Expression::parse, but allows for concurrent evaluation across multiple threads.
     * @param expr The expression to parse.
     */
    void parse(std::string const& expr){
        static_assert(EXPRESSION_POOL_SIZE > 0, "EXPRESSION_POOL_SIZE must be greater than 0");

        fullExpression = expr;

        // Parse each entry in the pool
        for (size_t i = 0; i < EXPRESSION_POOL_SIZE; ++i){
            pool[i] = std::make_unique<Expression>(expr);
        }

        // Calculate metadata and unique ID from first entry
        _isReturnableAsDouble = pool[0]->recalculateIsReturnableAsDouble();
        _isAlwaysTrue = pool[0]->recalculateIsAlwaysTrue();
    }

    // Pool of expressions
    std::array<std::unique_ptr<Expression>, EXPRESSION_POOL_SIZE> pool;

    //------------------------------------------
    // The following variables are shared across all pool entries
    // But placed here for easy access, without disturbing the pool.

    // Full expression string that was parsed
    std::string fullExpression;

    // Storing info about the expression's returnability
    bool _isReturnableAsDouble = false;

    // If the expression is just "1", meaning always true
    bool _isAlwaysTrue = false;

    /**
     * @brief Gets the thread index for the current thread, used to access the corresponding pool entry.
     * @return The thread index for the current thread, in the range [0, EXPRESSION_POOL_SIZE).
     */
    static size_t threadIndex() {
        thread_local size_t threadIndex = Data::JsonScope::assignThreadIndex() % EXPRESSION_POOL_SIZE;
        return threadIndex;
    }
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_POOL_HPP
