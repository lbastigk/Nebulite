/**
 * @file Expression.hpp
 * 
 * This file contains the definition of the Expression class, which is responsible for parsing and evaluating expressions within the Nebulite engine.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP

//------------------------------------------
// Includes

// General
#include "string"
#include <memory>
#include <deque>

// External
#include <tinyexpr.h>

// Nebulite
#include "Interaction/Logic/VirtualDouble.hpp"
#include "Utility/DocumentCache.hpp"
#include "Utility/Capture.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @class Nebulite::Interaction::Logic::Expression
 * @brief The Expression class is responsible for parsing and evaluating expressions.
 * 
 * It supports variable registration, expression compilation, and evaluation.
 * 
 * Expressions can be parsed from a string format and evaluated against JSON documents.
 * Expressions are a mix of evaluations, variables and text:
 * 
 * e.g.: 
 * "This script took {global.time.t} Seconds"
 * "The rounded value is: $03.2f( {global.value} )"
 */
class Expression {
public:
    Expression();

    ~Expression();

    /**
     * @brief Parses a given expression string with a constant reference to the document cache and the self and global JSON objects.
     * 
     * @param expr The expression string to parse.
     * @param documentCache The document cache to use for variable resolution.
     * @param self The JSON object representing the "self" context.
     * @param global The JSON object representing the "global" context.
     */
    void parse(std::string const& expr, Nebulite::Utility::DocumentCache* documentCache, Nebulite::Utility::JSON* self, Nebulite::Utility::JSON* global);

    /**
     * @brief Checks if the expression can be returned as a double.
     * 
     * e.g.:
     * "1 + 1"   is not returnable as double, as its just text
     * "$(1+1)"  is returnable as double, as it evaluates to 2
     * "$i(1+1)" is not returnable as double, due to the casting
     * 
     * An expression needs to consist of a single eval component with no cast to be returnable as double.
     * 
     * @return True if the expression can be returned as a double, false otherwise.
     */
    bool isReturnableAsDouble(){
        return _isReturnableAsDouble;
    }

    /**
     * @brief Checks if the expression is always true (i.e., "1").
     * @return True if the expression is always true, false otherwise.
     */
    bool isAlwaysTrue(){
        return _isAlwaysTrue;
    }

    /**
     * @brief Evaluates the expression as a double.
     * 
     * @param current_other The JSON object `other` to evaluate against.
     * @return The evaluated double value.
     */
    double evalAsDouble(Nebulite::Utility::JSON* current_other);
    
    /**
     * @brief Evaluates the expression as a string.
     * 
     * @param current_other The JSON object `other` to evaluate against.
     * @param max_recursion_depth The maximum recursion depth to prevent infinite loops in nested evaluations.
     * @return The evaluated string value.
     */
    std::string eval(Nebulite::Utility::JSON* current_other, uint16_t max_recursion_depth = 10);

    /**
     * @brief Gets the full expression string that was parsed.
     * 
     * @return The full expression string.
     */
    std::string const* getFullExpression() const noexcept {return &fullExpression;}

    /**
     * @brief Forcefully sets the unique ID for the expression.
     * Be careful when using this, as it might lead to issues with virtualDouble tracking!
     * This is only used when the id was calculated externally, e.g. in ExpressionPool.
     * @param id The unique ID to set.
     */
    void setUniqueId(uint64_t id){
        uniqueId = id;
    }

    //------------------------------------------
    // Helpers for recalculating expression info
    // helpful for expressionpool to reduce the amount of parsing needed

    /**
     * @brief Recalculates whether the expression is returnable as a double.
     * @return True if the expression can be returned as a double, false otherwise.
     */
    bool recalculateIsReturnableAsDouble();

    /**
     * @brief Recalculates whether the expression is always true (i.e., "1").
     * @return True if the expression is always true, false otherwise.
     */
    bool recalculateIsAlwaysTrue();

private:
    /**
     * @brief link to the remanent self context
     */
    Nebulite::Utility::JSON* self = nullptr;

    /**
     * @brief link to the remanent global context
     */
    Nebulite::Utility::JSON* global = nullptr;

    /**
     * @brief link to the non-remanent document cache
     */
    Nebulite::Utility::DocumentCache* globalCache = nullptr;

    /**
     * @struct Nebulite::Interaction::Logic::Expression::Component
     * @brief Represents a single component in an expression, such as a variable, evaluation, or text.
     * 
     * This struct holds information about a specific part of the expression,
     * including its type, source, and any associated metadata.
     */
    struct Component {
        /**
         * @enum Nebulite::Interaction::Logic::Expression::Component::Type
         * @brief Represents the type of an expression component.
         */
        enum Type {
            variable,   // outside $<cast>(...), Starts with self, other, global or a dot for link, represents a variable reference, outside of an evaluatable context
            eval,       // inside $<cast>(...), represents an evaluatable expression
            text        // outside of a $<cast>(...), not a variable reference, Represents a plain text string
        } type = Type::text;

        /**
         * @enum Nebulite::Interaction::Logic::Expression::Component::From
         * @brief Represents the source of a variable reference.
         */
        enum From {
            self,       // Using the "self" document for expression evaluation
            other,      // Using the "other" document for expression evaluation
            global,     // Using the "global" document for expression evaluation
            resource,   // Using a document from the document cache for expression evaluation
            None        // No context given for evaluation 
        } from = From::None; // Default to None

        /**
         * @enum Nebulite::Interaction::Logic::Expression::Component::CastType
         * @brief Represents the type of cast to apply to an expression component.
         */
        enum CastType {
            none,       // No cast -> using pure string
            to_int,     // Cast to integer
            to_double   // Cast to double
        } cast = CastType::none; // Default to none

        /**
         * @struct Nebulite::Interaction::Logic::Expression::Component::Formatter
         * @brief Represents formatting options for the component.
         */
        struct Formatter {
            /**
             * @brief Whether to pad with leading zeros.
             */
            bool leadingZero = false;

            /**
             * @brief The alignment width of the component.
             * 
             * -1 means no formatting.
             */
            int alignment = -1;

            /**
             * @brief The precision of the component.
             * 
             * -1 means no formatting.
             */
            int precision = -1;
        } formatter;

        /**
         * @brief Holds the string representation of the component.
         * 
         * Depending on context Either:
         * 
         * - The Expression to evaluate, with formatting specifiers removed
         * 
         * - The pure text
         * 
         * - The variable key, with no context stripped
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
        ~Component(){
            te_free(expression);
        }

        // Delete copy constructor/assignment
        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        // Move constructor
        Component(Component&& other) noexcept
            : type(other.type), from(other.from), cast(other.cast),
            formatter(other.formatter), str(std::move(other.str)), key(std::move(other.key)),
            expression(other.expression)
        {
            other.expression = nullptr;
        }

        // Move assignment
        Component& operator=(Component&& other) noexcept {
            if (this != &other){
                te_free(expression);
                type = other.type;
                from = other.from;
                cast = other.cast;
                formatter = other.formatter;
                str = std::move(other.str);
                key = std::move(other.key);
                expression = other.expression;
                other.expression = nullptr;
            }
            return *this;
        }
    };

    /**
     * @struct Nebulite::Interaction::Logic::Expression::VirtualDoubleLists
     * @brief Holds lists of VirtualDouble entries for different contexts.
     */
    struct VirtualDoubleLists{
        using vd_list = std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>>;

        /**
         * @brief Holds all virtual double entries for the self context.
         */
        vd_list self;

        /**
         * @brief Holds all virtual double entries for the other context.
         */
        vd_list other;

        /**
         * @brief Holds all virtual double entries for the global context.
         */
        vd_list global;

        /**
         * @brief Holds all virtual double entries for the resource context.
         */
        vd_list resource;
    } virtualDoubles;

    /**
     * @brief A collection of custom functions for TinyExpr
     * 
     * Make sure to register all functions with TinyExpr in Nebulite::Interaction::Logic::Expression::reset
     */
    class expr_custom{
    public:
        /**
         * @brief Epsilon value for floating-point comparisons.
         */
        static constexpr double epsilon = 1e-9;

        // Logical comparison functions
        static double gt(double a, double b){return a > b;}
        static double lt(double a, double b){return a < b;}
        static double geq(double a, double b){return a >= b;}
        static double leq(double a, double b){return a <= b;}
        static double eq(double a, double b){
            return (std::fabs(a - b) < epsilon);
        }
        static double neq(double a, double b){
            return !(std::fabs(a - b) > epsilon);
        }

        // Logical gate functions
        static double logical_not(double a){
            return !(std::fabs(a) > epsilon);
        }

        static double logical_and(double a, double b){
            bool aLogical = (std::fabs(a) > epsilon);
            bool bLogical = (std::fabs(b) > epsilon);
            return static_cast<double>(aLogical && bLogical);
        }
        static double logical_or(double a, double b){
            bool aLogical = (std::fabs(a) > epsilon);
            bool bLogical = (std::fabs(b) > epsilon);
            return static_cast<double>(aLogical || bLogical);
        }
        static double logical_xor(double a, double b){
            bool aLogical = (std::fabs(a) > epsilon);
            bool bLogical = (std::fabs(b) > epsilon);
            return static_cast<double>(aLogical != bLogical);
        }

        static double logical_nand(double a, double b){
            bool aLogical = (std::fabs(a) > epsilon);
            bool bLogical = (std::fabs(b) > epsilon);
            return !(aLogical && bLogical);
        }
        static double logical_nor(double a, double b){
            bool aLogical = (std::fabs(a) > epsilon);
            bool bLogical = (std::fabs(b) > epsilon);
            return static_cast<double>(!(aLogical || bLogical));
        }
        static double logical_xnor(double a, double b){
            bool aLogical = (std::fabs(a) > epsilon);
            bool bLogical = (std::fabs(b) > epsilon);
            return static_cast<double>(!( (aLogical || bLogical) && !(aLogical && bLogical) ));
        }

        // Other logical functions
        static double to_bipolar(double a){
            return (std::fabs(a) > epsilon) ? 1 : -1;
        }

        // Mapping functions
        static double map(double value, double in_min, double in_max, double out_min, double out_max){
            if(std::fabs(in_max - in_min) < epsilon) return out_min; // Prevent division by zero
            if(value < in_min) return out_min;
            if(value > in_max) return out_max;
            return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }
        static double constrain(double value, double min, double max){
            if(value < min) return min;
            if(value > max) return max;
            return value;
        }

        // More mathematical functions
        static double sgn(double a){return std::copysign(1.0, a);}
    };

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
     * 
     * - Clears all components
     * - Clears all variables and re-registers standard functions
     * - Clears all virtual double entries
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
    std::vector<std::shared_ptr<std::string>> te_names;      // Names of variables for TinyExpr evaluation

    /**
     * @brief Collection of all registered variables and functions
     */
    std::vector<te_variable> te_variables;   // Variables for TinyExpr evaluation

    /**
     * @brief Reference to the resource context
     */
    Nebulite::Utility::DocumentCache* documentCache = nullptr;

    /**
     * @brief The unique ID from globalspace for this expression string
     */
    uint64_t uniqueId;

    //------------------------------------------
    // Helper functions

    /**
     * @brief Compiles an component, if its of type Expression
     * 
     * @param component The component to potentially compile
     */
    void compileIfExpression(std::shared_ptr<Component>& component);

    /**
     * @brief Registers a variable with the given name and key in the context of the component.
     * 
     * Makes sure to only register variables that are not already registered.
     * 
     * @param te_name The name of the variable as used in TinyExpr.
     * @param key The key in the JSON document that the variable refers to.
     * @param context The context from which the variable is being registered.
     */
    void registerVariable(std::string te_name, std::string key, Component::From context);

    /**
     * @brief used to strip any context prefix from a key
     * 
     * Removes the beginning, if applicable:
     *
     * - `self.`
     *
     * - `other.`
     *
     * - `global.`
     *
     * Does not remove the beginning context for resource variables, 
     * as the beginning is needed for the link.
     * 
     * @param key The key to strip the context from.
     * 
     * @return The key without its context prefix.
     */
    static std::string stripContext(std::string const& key);

    /**
     * @brief Gets the context from a key before it's stripped
     *
     * If the key doesnt start with `self.`, `other.`, or `global.`, it is considered a resource variable.
     * 
     * @param key The key to get the context from.
     * 
     * @return The context of the key.
     */
    static Component::From getContext(std::string const& key);

    /**
     * @brief Parses the given expression into a series of components.
     * 
     * @param expr The expression string to parse.
     * @param components The vector to populate with the parsed components.
     */
    void parseIntoComponents(std::string const& expr, std::vector<std::shared_ptr<Component>>& components);

    /**
     * @brief Reads the formatter string from a string and parses it intro the component.
     * 
     * @param component The component to populate with the parsed formatter.
     * @param formatter The formatter string to parse.
     */
    static void readFormatter(std::shared_ptr<Component> const& component, std::string const& formatter);

    /**
     * @brief Used to parse a string token of type "eval" into an component.
     * 
     * - Parses the token on the assumption that it is of type "eval".
     * 
     * - Populates the current component with the parsed information.
     * 
     * - Pushes the current component onto the components vector.
     * 
     * @param token The token to parse.
     * @param components The vector to push the component onto.
     */
    void parseTokenTypeEval(std::string const& token, std::vector<std::shared_ptr<Component>>& components);

    /**
     * @brief Used to parse a string token of type "text" into an component.
     * 
     * - Parses the token on the assumption that it is of type "text".
     * 
     * - Populates the current component with the parsed information.
     * 
     * - Pushes the current component onto the components vector.
     * 
     * @param token The token to parse.
     * @param components The vector to push the component onto.
     */
    void parseTokenTypeText(std::string const& token, std::vector<std::shared_ptr<Component>>& components);

    /**
     * @brief Prints a compilation error message to cerr
     * 
     * Includes tips for fixing the error.
     */
    void printCompileError(const std::shared_ptr<Component>& component, const int error);

    /**
     * @brief Updates caches
     */
    void updateCaches(Nebulite::Utility::JSON* reference);

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for "other" context variables.
     *
     * This function checks if the current "other" reference JSON document contains a cached, ordered list of double pointers
     * corresponding to all variables referenced by this Expression in the "other" context. If the cache entry does not exist,
     * it is created and populated for fast indexed access during expression evaluation.
     *
     * This caching mechanism is critical for Nebulite's high-performance expression system, as it avoids repeated
     * string lookups and pointer resolutions for variables in other objects, enabling near O(1) access.
     *
     * @param reference The JSON document representing the "other" context for variable resolution.
     * @return A pointer to the ordered vector of double pointers for the referenced "other" variables.
     * @brief Ensures that there is a cache component for the given other JSON document and expression.
     * 
     * @param current_other The other JSON document to ensure a cache component for.
     * 
     * @return A pointer to the vector of double pointers for the expression in the other document.
     */
    odpvec* ensureOtherOrderedCacheList(Nebulite::Utility::JSON* reference);

    /**
     * @brief Handles the evaluation of a variable component.
     * 
     * @param token The string to populate with the evaluated value.
     * @param component The component to evaluate.
     * @return True if the evaluation was successful, false otherwise.
     */
    bool handleComponentTypeVariable(std::string& token, const std::shared_ptr<Component>& component, Nebulite::Utility::JSON* current_other, uint16_t max_recursion_depth);

    /**
     * @brief Handles the evaluation of an eval component.
     */
    void handleComponentTypeEval(std::string& token, const std::shared_ptr<Component>& component);
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_HPP