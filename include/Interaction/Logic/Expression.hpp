/**
 * @file Expression.hpp
 * @brief This file contains the definition of the Expression class, which is responsible for parsing and evaluating expressions within the Nebulite engine.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>

// External
#include <tinyexpr.h>

// Nebulite
#include "Data/Document/JsonScopeBase.hpp"
#include "Interaction/Logic/VariableNameGenerator.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class JsonScope;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class ScopedKey;
class ScopedKeyView;
} // namespace Nebulite::Data

namespace Nebulite::Interaction {
class Context;
} // namespace Nebulite::Interaction

namespace Nebulite::Interaction::Logic {
class VirtualDouble;
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
 *          "This script took {global.time.t} Seconds"
 *          "The rounded value is: $03.2f( {global.value} )"
 *          Supports anti-evaluation formatting with {! ... }:
 *          Any variable wrapped in {!...} instead of {...} will be treated as pure text and will not be evaluated
 * @todo Add support for marrying contexts into a single data structure for transformations
 *       Example: If we have a matrix transformation module, multiplying matrices
 *       from different contexts can be difficult, as we need to somehow copy them into one context first.
 *       With context marrying, we could have a single context that encompasses all variables from self, other and global, with some sort of prefix to differentiate them.
 *       {all.|matMultiply self.matrix other.matrix} could then be evaluated directly without needing to copy variables into a new context first.
 *       Unless we also implement scope marrying, we will have to copy a lot of data here. Perhaps a selfother combined context is helpful for faster evaluation:
 *       {so.|matMultiply self.matrix other.matrix} could then be evaluated directly without needing to copy global variables, which is typically the largest portion of variables, into a new context first.
 */
class Expression {
public:
    /**
     * @brief Constructs and parses a given expression string with a constant reference to the document cache and the self and global JSON objects.
     * @param expr The expression string to parse.
     * @param selfScope The JSON object representing the "self" context.
     */
    explicit Expression(std::string const& expr, Data::JsonScopeBase& selfScope);

    explicit Expression(std::string const& expr, Execution::Domain const& selfDomain);

    ~Expression();

    // disable copying
    Expression(Expression const&) = delete;
    Expression& operator=(Expression const&) = delete;

    // enable moving
    Expression(Expression&&) noexcept = default;

    /**
     * @brief Standard maximum recursion depth for nested expression evaluations.
     */
    static constexpr uint16_t standardMaximumRecursionDepth = 10;

    /**
     * @brief Checks if the expression can be returned as a double.
     * @details e.g.:
     *          "1 + 1"   is not returnable as double, as its just text
     *          "$(1+1)"  is returnable as double, as it evaluates to 2
     *          "$i(1+1)" is not returnable as double, due to the casting
     *          An expression needs to consist of a single eval component with no cast to be returnable as double.
     * @return True if the expression can be returned as a double, false otherwise.
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

    /**
     * @brief Evaluates the expression as a JSON object.
     * @param current_other The JSON object `other` to evaluate against.
     * @param max_recursion_depth The maximum recursion depth to prevent infinite loops in nested evaluations.
     * @return The evaluated JSON object.
     */
    Data::JSON evalAsJson(Data::JsonScopeBase& current_other, uint16_t const& max_recursion_depth = standardMaximumRecursionDepth) const ;

    /**
     * @brief Evaluates the expression as a double.
     * @param current_other The JSON object `other` to evaluate against.
     * @return The evaluated double value.
     */
    double evalAsDouble(Data::JsonScopeBase& current_other) const ;

    /**
     * @brief Evaluates the expression as a string.
     * @param current_other The JSON object `other` to evaluate against.
     * @param max_recursion_depth The maximum recursion depth to prevent infinite loops in nested evaluations.
     * @return The evaluated string value.
     */
    std::string eval(Data::JsonScopeBase& current_other, uint16_t const& max_recursion_depth = standardMaximumRecursionDepth) const ;

    /**
     * @brief Gets the full expression string that was parsed.
     *
     * @return The full expression string.
     */
    [[nodiscard]] std::string const* getFullExpression() const noexcept { return &fullExpression; }

    /**
     * @brief Forcefully sets the unique ID for the expression.
     * @details Be careful when using this, as it might lead to issues with virtualDouble tracking!
     *          This is only used when the id was calculated externally, e.g. in ExpressionPool.
     * @param id The unique ID to set.
     */
    void setUniqueId(uint64_t const id) {
        uniqueId = id;
    }

    //------------------------------------------
    // Helpers for recalculating expression info
    // helpful for ExpressionPool to reduce the amount of parsing needed

    /**
     * @brief Recalculates whether the expression is returnable as a double.
     * @return True if the expression can be returned as a double, false otherwise.
     */
    [[nodiscard]] bool recalculateIsReturnableAsDouble() const;

    /**
     * @brief Recalculates whether the expression is always true (i.e., "1").
     * @return True if the expression is always true, false otherwise.
     */
    [[nodiscard]] bool recalculateIsAlwaysTrue() const;

    //------------------------------------------
    // Static one-time evaluation

    // TODO: Reduce from Context to ContextScopeBase

    // With context evaluation

    /**
     * @brief Evaluates a given expression string with a constant reference to the context.
     * @param input The expression string to evaluate.
     * @param context The context containing the self, other, and global JSON objects.
     * @return The evaluated string value.
     */
    static std::string eval(std::string const& input, Context const& context);

    /**
     * @brief Evaluates a given expression string as a double with a constant reference to the context.
     * @param input The expression string to evaluate.
     * @param context The context containing the self, other, and global JSON objects.
     * @return The evaluated double value.
     */
    static double evalAsDouble(std::string const& input, Context const& context);

    /**
     * @brief Evaluates a given expression string as a boolean with a constant reference to the context.
     * @param input The expression string to evaluate.
     * @param context The context containing the self, other, and global JSON objects.
     * @return The evaluated boolean value.
     */
    static bool evalAsBool(std::string const& input, Context const& context);

    /**
     * @brief Evaluates a given expression string as a JSON object with a constant reference to the context.
     * @param input The expression string to evaluate.
     * @param context The context containing the self, other, and global JSON objects.
     * @return The evaluated JSON object.
     */
    static Data::JSON evalAsJson(std::string const& input, Context const& context);

    // Global-only evaluation (both self and other context are empty documents)

    /**
     * @brief Evaluates a given expression string with global context only.
     * @param input The expression string to evaluate.
     * @return The evaluated string value.
     */
    static std::string eval(std::string const& input);

    /**
     * @brief Evaluates a given expression string as a double with global context only.
     * @param input The expression string to evaluate.
     * @return The evaluated double value.
     */
    static double evalAsDouble(std::string const& input);

    /**
     * @brief Evaluates a given expression string as a boolean with global context only.
     * @param input The expression string to evaluate.
     * @return The evaluated boolean value.
     */
    static bool evalAsBool(std::string const& input);

    /**
     * @brief Generates a unique ID for a given expression string.
     * @param expression The expression string to generate an ID for.
     * @return The generated unique ID.
     */
    static uint64_t generateUniqueId(std::string const& expression);

private:
    /**
     * @brief Parses a given expression string with a constant reference to the document cache and the self and global JSON objects.
     * @param expr The expression string to parse.
     */
    void parse(std::string const& expr);

    /**
     * @brief The reference for context self stays the same throughout the expression's lifetime.
     * @details This allows us to cache variables from self directly, not reloading needed
     */
    Data::JsonScopeBase& self;

    /**
     * @brief Generates short variable names for tinyexpr variables.
     */
    VariableNameGenerator varNameGen;

    /**
     * @struct Nebulite::Interaction::Logic::Expression::Component
     * @brief Represents a single component in an expression, such as a variable, evaluation, or text.
     * @details Holds information about a specific part of the expression,
     *          including its type, source, and any associated metadata.
     */
    class Component {
    public:
        /**
         * @enum Nebulite::Interaction::Logic::Expression::Component::Type
         * @brief Each component can be of type variable, eval or text that differ in how they are evaluated.
         */
        enum class Type : uint8_t {
            variable, // outside $<cast>(...), Starts with self, other, global or a dot for link, represents a variable reference, outside an evaluatable context
            eval, // inside $<cast>(...), represents an evaluatable expression
            text // outside of a $<cast>(...), not a variable reference, Represents a plain text string
        } type = Type::text;

        /**
         * @enum Nebulite::Interaction::Logic::Expression::Component::ContextType
         * @brief Represents the source of a variable reference.
         */
        enum class ContextType : uint8_t {
            self, // Using the "self" document for expression evaluation
            other, // Using the "other" document for expression evaluation
            global, // Using the "global" document for expression evaluation
            resource, // Using a document from the document cache for expression evaluation
            None // No context given for evaluation
        } contextType = ContextType::None; // Default to None

        /**
         * @enum Nebulite::Interaction::Logic::Expression::Component::CastType
         * @brief Represents the type of cast to apply to an expression component.
         */
        enum class CastType : uint8_t {
            none, // No cast -> using pure string
            to_int, // Cast to integer
            to_double // Cast to double
        } cast = CastType::none; // Default to none

        /**
         * @struct Nebulite::Interaction::Logic::Expression::Component::Formatter
         * @brief Represents formatting options for the component.
         */
        struct Formatter {
            bool leadingZero = false; // If true, pad with leading zeros
            int alignment = -1; // The alignment width of the component. -1 means no formatting.
            int precision = -1; // The precision of the component. -1 means no formatting.
        } formatter;

        /**
         * @brief Holds the string representation of the component.
         *        Depending on context Either:
         *        - The Expression to evaluate, with formatting specifiers removed
         *        - The pure text
         *        - The variable key, with no context stripped
         */
        std::string str;

        /**
         * @brief Holds the context-stripped key of the component, if it's of type variable.
         */
        std::string key;

        /**
         * @brief Pointer to the tinyexpr representation of the expression.
         */
        te_expr* expression = nullptr;

        /**
         * @brief Default constructor for Component.
         */
        Component() = default;

        /**
         * @brief Destructor to clean up allocated resources.
         */
        ~Component() {
            te_free(expression);
        }

        // disable copying
        Component(Component const&) = delete;
        Component& operator=(Component const&) = delete;

        // enable moving
        Component(Component&& other) noexcept ;
        Component& operator=(Component&& other) noexcept ;

        //------------------------------------------
        // Component handling methods

        /**
         * @brief Handles the evaluation of a variable component.
         * @param token The string to populate with the evaluated value.
         * @param selfScope The scope `self` to evaluate against.
         * @param otherScope The scope `other` to evaluate against.
         * @param maximumRecursionDepth The maximum recursion depth for nested evaluations.
         * @return True if the evaluation was successful, false otherwise.
         */
        bool handleComponentTypeVariable(std::string& token, Data::JsonScopeBase& selfScope, Data::JsonScopeBase& otherScope, uint16_t const& maximumRecursionDepth) const ;

        /**
         * @brief Handles the evaluation of a variable component.
         * @param token The JSON object to populate with the evaluated value.
         * @param selfScope The scope `self` to evaluate against.
         * @param otherScope The scope `other` to evaluate against.
         * @param maximumRecursionDepth The maximum recursion depth for nested evaluations.
         * @return True if the evaluation was successful, false otherwise.
         */
        bool handleComponentTypeVariable(Data::JSON& token, Data::JsonScopeBase& selfScope, Data::JsonScopeBase& otherScope, uint16_t const& maximumRecursionDepth) const ;

        /**
         * @brief Handles the evaluation of an eval component.
         * @param token The string to populate with the evaluated value.
         */
        void handleComponentTypeEval(std::string& token) const ;
    };

    /**
     * @struct Nebulite::Interaction::Logic::Expression::VirtualDoubleLists
     * @brief Holds lists of VirtualDouble entries for different contexts.
     */
    struct VirtualDoubleLists {
        using vd_list = std::vector<std::shared_ptr<VirtualDouble>>;

        /**
         * @brief Contains virtual doubles that are remanent
         *        Meaning they can be cached directly from the document
         */
        struct RemanentContext {
            vd_list self; // Variables from context self without transformations or multi-resolve
            vd_list global; // Variables from context global without transformations or multi-resolve
        } remanent;

        /**
         * @brief Contains virtual doubles that are non-remanent
         *        Meaning they need to be updated on each eval
         */
        struct NonRemanentContext {
            vd_list self; // Variables from context self with transformations or multi-resolve
            vd_list other; // Variables from context other with transformations or multi-resolve
            vd_list otherUnStable; // Variables from context other that are unstable (with transformations or multi-resolve)
            vd_list global; // All variables from context global
            vd_list resource; // All variables from context resource
            vd_list none; // Variables with no context
        } nonRemanent;
    } virtualDoubles;

    /**
     * @brief Storing info about the expression's returnability
     */
    bool _isReturnableAsDouble;

    /**
     * @brief Storing info about the expression's always-true state
     */
    bool _isAlwaysTrue;

    /**
     * @brief Resets the expression to its initial state.
     * @details This function:
     *          - Clears all components
     *          - Clears all variables and re-registers standard functions
     *          - Clears all virtual double entries
     */
    void reset();

    /**
     * @brief Holds all parsed components from the expression.
     */
    std::vector<std::shared_ptr<Component>> components;

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

    /**
     * @brief The unique ID from globalspace for this expression string
     */
    uint64_t uniqueId;

    //------------------------------------------
    // Helper functions

    /**
     * @brief Compiles a component, if its of type Expression
     * @param component The component to potentially compile
     */
    void compileIfExpression(std::shared_ptr<Component> const& component) const;

    /**
     * @brief Registers a variable with the given name and key in the context of the component.
     *        Makes sure to only register variables that are not already registered.
     * @param te_name The name of the variable as used in TinyExpr.
     * @param key The key in the JSON document that the variable refers to.
     * @param context The context from which the variable is being registered.
     */
    void registerVariable(std::string te_name, std::string const& key, Component::ContextType const& context);

    /**
     * @brief used to strip any context prefix from a key
     * @details Removes the beginning, if applicable:
     *          - `self.`
     *          - `other.`
     *          - `global.`
     *          Does not remove the beginning context for resource variables,
     *          as the beginning is needed for the link.
     * @param key The key to strip the context from.
     * @return The key without its context prefix.
     */
    static std::string stripContext(std::string const& key);

    /**
     * @brief Gets the context from a key before it's stripped
     * @details If the key doesn't start with `self.`, `other.`, or `global.`, it is considered a resource variable.
     * @param key The key to get the context from.
     * @return The context of the key.
     */
    static Component::ContextType getContext(std::string const& key);

    /**
     * @brief Parses the given expression into a series of components.
     * @param expr The expression string to parse.
     */
    void parseIntoComponents(std::string const& expr);

    /**
     * @brief Reads the formatter string from a string and parses it intro the component.
     * @param component The component to populate with the parsed formatter.
     * @param formatter The formatter string to parse.
     */
    static void readFormatter(std::shared_ptr<Component> const& component, std::string const& formatter);

    /**
     * @brief Used to parse a string token of type "eval" into a component.
     * @details Tasks:
     *          - Parses the token on the assumption that it is of type "eval".
     *          - Populates the current component with the parsed information.
     *          - Pushes the current component onto the components vector.
     * @param token The token to parse.
     */
    void parseTokenTypeEval(std::string const& token);

    /**
     * @brief Used to parse a string token of type "text" into a component.
     * @details Tasks:
     *          - Parses the token on the assumption that it is of type "text".
     *          - Populates the current component with the parsed information.
     *          - Pushes the current component onto the components vector.
     * @param token The token to parse.
     */
    void parseTokenTypeText(std::string const& token);

    /**
     * @brief Prints a compilation error message to cerr, includes tips for fixing the error.
     */
    void printCompileError(std::shared_ptr<Component> const& component, int const& error) const;

    /**
     * @brief Updates caches
     */
    void updateCaches(Data::JsonScopeBase& reference) const ;
};

} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP
