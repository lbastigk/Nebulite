#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint> // NOLINT
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// External
#include <tinyexpr.h>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/VariableNameGenerator.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
class ScopedKey;
class ScopedKeyView;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Logic {
class ExpressionComponent;
class LinkedNumericValue;
} // namespace Nebulite::Interaction::Logic

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @class Nebulite::Interaction::Logic::Expression
 * @brief The Expression class is responsible for parsing and evaluating expressions.
 * @details It supports variable registration, expression compilation, and evaluation.
 *          Expressions can be parsed from a string format and evaluated against JSON documents.
 *          Expressions are a mix of evaluations, variables and text:
 *          e.g.:
 *          "This script took {global:time.t} Seconds"
 *          "The rounded value is: $03.2f( {global:value} )"
 *          Supports explicit evaluation delay formatting with {n! ... }:
 *          Any variable wrapped in {!...} instead of {...} will be treated as pure text and will not be evaluated
 */
class Expression {
public:
    /**
     * @brief Constructs and parses a given expression string with a constant reference to the document cache and the self and global JSON objects.
     * @param expr The expression string to parse.
     */
    explicit Expression(std::string_view expr);

    ~Expression();

    // disable copying
    Expression(Expression const&) = delete;
    Expression& operator=(Expression const&) = delete;

    // enable moving
    Expression(Expression&&) noexcept = default;
    Expression& operator=(Expression&&) noexcept = default;

    /**
     * @brief Standard maximum recursion depth for nested expression evaluations.
     */
    static constexpr std::size_t standardRecursionDepth = 10;

    /**
     * @brief Checks if the expression can be returned as a double without losing information.
     * @details e.g.:
     *          "1 + 1"   is not returnable as double, as its just text
     *          "$(1+1)"  is returnable as double, as it evaluates to 2
     *          "$i(1+1)" is not returnable as double, due to the casting
     *          An expression needs to consist of a single eval component
     *          with no cast and no formatting to be returnable as double.
     * @return True if the expression can be returned as a double, false otherwise.
     */
    [[nodiscard]] bool isReturnableAsDouble() const noexcept {
        return evaluationInfo.returnableAsDouble;
    }

    /**
     * @brief Checks if the expression can be returned as into without losing information.
     * @details An expression needs to consist of a single eval component
     *          with cast to int and no formatting to be returnable as int.
     * @return True if the expression can be returned as int, false otherwise
     */
    [[nodiscard]] bool isReturnableAsInt() const noexcept {
        return evaluationInfo.returnableAsInt;
    }

    /**
     * @brief Checks if the expression can be returned as a string.
     * @details This is almost always the case. The only exception is an expression with only one variable,
     *          e.g. "{global:var}" or "{self:arr}"
     * @return True if the expression can be returned as string, false otherwise.
     */
    [[nodiscard]] bool isReturnableAsString() const noexcept {
        return evaluationInfo.returnableAsString;
    }

    /**
     * @brief Checks if the expression is always true (i.e., "1").
     * @return True if the expression is always true, false otherwise.
     */
    [[nodiscard]] bool isAlwaysTrue() const noexcept {
        return evaluationInfo.alwaysTrue;
    }

    //------------------------------------------
    // Actual evaluation functions

    [[nodiscard]] std::string eval(ContextScope const& context, std::size_t recursionDepth = standardRecursionDepth) const ;

    [[nodiscard]] double evalAsDouble(ContextScope const& context) const ;

    [[nodiscard]] std::int64_t evalAsInt(ContextScope const& context) const ;

    [[nodiscard]] bool evalAsBool(ContextScope const& context) const ;

    [[nodiscard]] Data::JSON evalAsJson(ContextScope const& context, std::size_t recursionDepth = standardRecursionDepth) const ;

    //------------------------------------------
    // Static functions for one-time evaluation

    static std::string eval(std::string_view input, ContextScope const& context);

    static double evalAsDouble(std::string_view input, ContextScope const& context);

    static bool evalAsBool(std::string_view input, ContextScope const& context);

    static Data::JSON evalAsJson(std::string_view input, ContextScope const& context);

    //------------------------------------------
    // Other helpers

    /**
     * @brief Gets the full expression string that was parsed.
     * @return The full expression string.
     */
    [[nodiscard]] std::string const& getFullExpression() const noexcept { return fullExpression; }

    /**
     * @brief Recalculates whether the expression is returnable as a double.
     * @return True if the expression can be returned as a double, false otherwise.
     */
    [[nodiscard]] bool recalculateIsReturnableAsDouble() const;

    [[nodiscard]] bool recalculateIsReturnableAsInt() const;

    [[nodiscard]] bool recalculateIsReturnableAsString() const;

    /**
     * @brief Recalculates whether the expression is always true (i.e., "1").
     * @return True if the expression is always true, false otherwise.
     */
    [[nodiscard]] bool recalculateIsAlwaysTrue() const;

private:
    /**
     * @struct Nebulite::Interaction::Logic::Expression::LinkedNumericValueLists
     * @brief Holds lists of LinkedNumericValue entries for different contexts.
     */
    struct LinkedNumericValueLists {
        using lnvList = std::vector<std::shared_ptr<LinkedNumericValue>>;

        // Linkable as external cache, no multi-resolve or transformations
        // This works by Caching the first context used. If the new context address matches the first,
        // we can use the stable vd_list and simply copy double values.
        // Otherwise, we need to retrieve them from a document first, which is expensive
        struct Stable {
            lnvList self; // Variables from context self
            lnvList other; // Variables from context other
            lnvList global; // Variables from context global
        } stable;

        // With multi-resolve or transformations, key needs to be resolved each time
        struct Unstable {
            lnvList self; // Variables from context self with transformations or multi-resolve
            lnvList other; // Variables from context other with transformations or multi-resolve
            lnvList local; // Variables from context marrying: self and other
            lnvList global; // Variables from context global with transformations or multi-resolve
            lnvList full; // Variables from context marrying: self, other and global
            lnvList resource; // Variables from context resource with transformations or multi-resolve
            lnvList none; // Variables with no context with transformations or multi-resolve
        } unstable;

        /**
         * @brief Registers a variable
         * @param contextType The context to register for
         * @param key The key to register
         * @return Pointer to the registered variable
         */
        double* registerVariable(ContextDeriver::TargetType contextType, std::string_view key);
    } linkedNumericValues;

    /**
     * @brief Generates short variable names for tinyexpr variables.
     * @details Short names might improve performance, but the main concern is
     *          that full variable names could contain characters that tinyexpr does not like
     */
    VariableNameGenerator varNameGen;

    /**
     * @brief Info about the expressions evaluation-ability
     * @details Some expressions are not always castable to types like numeric values or strings
     *          without the loss of information
     */
    struct EvaluationInfo {
        /**
         * @brief Only true if the expression consists of a single component of type eval
         */
        bool returnableAsDouble = false;

        /**
         * @brief Only true if the expression consists of a single component of type eval with cast to int
         */
        bool returnableAsInt = false;

        /**
         * @brief Only false if the expression consists of a single component of type variable
         * @details This is because retrieving values without any additional text etc. has no implicit cast to string
         *          A single value could hold more complex types: "{global:someObject}",
         *          whereas "My value is: {global:value}" has an implicit cast to a string.
         */
        bool returnableAsString = false;

        /**
         * @brief True if the expression is a simple non-zero numeric value to evaluate
         */
        bool alwaysTrue = false;
    } evaluationInfo;

    /**
     * @brief Holds all parsed components from the expression.
     */
    std::vector<std::shared_ptr<ExpressionComponent>> components;

    /**
     * @brief Holds the full expression as a string.
     */
    std::string fullExpression;

    /**
     * @brief Collection of all variable names
     */
    std::vector<std::shared_ptr<std::string>> te_names; // Names of variables for TinyExpr evaluation

    /**
     * @brief Collection of all registered variables and functions
     */
    std::vector<te_variable> te_variables; // Variables for TinyExpr evaluation

    //------------------------------------------
    // Core Helper functions

    /**
     * @brief Provides an empty JSON document that can be used as a context placeholder
     * @return The empty JSON document reference
     */
    static Data::JsonScope const& emptyDoc();

    /**
     * @brief Parses a given expression string with a constant reference to the document cache and the self and global JSON objects.
     * @param expr The expression string to parse.
     */
    void parse(std::string_view expr);

    /**
     * @brief Resets the expression to its initial state.
     * @details This function:
     *          - Clears all components
     *          - Clears all variables and re-registers standard functions
     *          - Clears all virtual double entries
     */
    void reset();

    /**
     * @brief Compiles a component, if its of type Expression
     * @param component The component to potentially compile
     */
    void compileIfExpression(std::shared_ptr<ExpressionComponent> const& component) const;

    /**
     * @brief Registers a variable with the given name and key in the context of the component.
     *        Makes sure to only register variables that are not already registered.
     * @param te_name The name of the variable as used in TinyExpr.
     * @param key The key in the JSON document that the variable refers to.
     * @param contextType The context from which the variable is being registered.
     */
    void registerVariable(std::string te_name, std::string_view key, ContextDeriver::TargetType contextType);

    /**
     * @brief Parses the given expression into a series of components.
     * @param expr The expression string to parse.
     */
    void parseIntoComponents(std::string_view expr);

    /**
     * @brief Used to parse a string token of type "eval" into a component.
     * @param token The token to parse.
     */
    void parseTokenTypeEval(std::string_view token);

    /**
     * @brief Used to parse a string token of type "variable" into a component.
     * @param token The token to parse
     */
    void parseTokenTypeVariable(std::string_view token);

    /**
     * @brief Used to parse a string token of type "text" into a component.
     * @param token The token to parse.
     */
    void parseTokenTypeText(std::string_view token);

    /**
     * @brief Prints a compilation error message to cerr, includes tips for fixing the error.
     */
    void printCompileError(std::shared_ptr<ExpressionComponent> const& component, int error) const;

    //------------------------------------------
    // Cache helper functions

    /**
     * @brief Updates caches to reflect current context
     * @param context The context to update caches for
     */
    void updateCaches(ContextScope const& context) const ;

    /**
     * @brief Updates the stable value caches based on the current context.
     * @details Either copies from first context via pointers (fast),
     *          or copies from the current context via json->get (slow)
     * @param context The current context to update stable value caches for
     */
    void updateStableValues(ContextScope const& context) const ;

    /**
    * @brief Updates the unstable value caches based on the current context.
    * @details Evaluates member and fetches value from the context
    * @param context The current context to update unstable value caches for
    */
    void updateUnstableValues(ContextScope const& context) const ;
};

} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP
