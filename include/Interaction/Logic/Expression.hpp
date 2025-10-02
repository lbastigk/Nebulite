/**
 * @file Expression.hpp
 * 
 * This file contains the definition of the Expression class, which is responsible for parsing and evaluating expressions within the Nebulite engine.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "string"
#include <memory>
#include <deque>

// External
#include "tinyexpr.h"

// Nebulite
#include "Interaction/Logic/VirtualDouble.hpp"
#include "Utility/DocumentCache.hpp"

//------------------------------------------
/**
 * @brief Enables the use of an external cache for double-values.
 * 
 * This means that double-values from inside expressions will use the JSON storage directly, if possible.
 * In order to use double-value-pointers from a JSON document, the document must be remanent:
 *
 * - The document is permanently accessible within the expression lifetime
 * 
 * - The document is permanently alive within the expression lifetime
 * 
 * - The document reference is constant within the expression lifetime
 *
 * This is only applicable for `self` and `global`.
 *
 * The `other` context changes frequently and is not suitable for external caching.
 * The `resource` context may be deloaded at any time, rendering double references invalid.
 */
#define use_external_cache 1

//------------------------------------------
namespace Nebulite {
namespace Interaction {
namespace Logic {
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
    void parse(const std::string& expr, Nebulite::Utility::DocumentCache& documentCache, Nebulite::Utility::JSON* self, Nebulite::Utility::JSON* global);

    /**
     * @brief Checks if the expression can be returned as a double.
     * 
     * e.g.:
     * "1 + 1"   is not returnable as double, as its just text
     * "$(1+1)"  is returnable as double, as it evaluates to 2
     * "$i(1+1)" is not returnable as double, due to the casting
     * 
     * An expression needs to consist of a single eval entry with no cast to be returnable as double.
     * 
     * @return True if the expression can be returned as a double, false otherwise.
     */
    bool isReturnableAsDouble();

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
     * @return The evaluated string value.
     */
    std::string eval(Nebulite::Utility::JSON* current_other);

    /**
     * @brief Gets the full expression string that was parsed.
     * 
     * @return The full expression string.
     */
    const std::string* getFullExpression() const noexcept {return &fullExpression;};

private:
    /**
     * @brief link to the remanentself context
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
     * @struct Nebulite::Interaction::Logic::Expression::Entry
     * @brief Represents a single entry in an expression.
     * 
     * This struct holds information about a specific part of the expression,
     * including its type, source, and any associated metadata.
     */
    struct Entry {
        /**
         * @enum Nebulite::Interaction::Logic::Expression::Entry::Type
         * @brief Represents the type of an expression entry.
         */
        enum Type {
            variable,   // outside $<cast>(...), Starts with self, other, global or a dot for link, represents a variable reference, outside of an evaluatable context
            eval,       // inside $<cast>(...), represents an evaluatable expression
            text        // outside of a $<cast>(...), not a variable reference, Represents a plain text string
        } type = Type::text;

        /**
         * @enum Nebulite::Interaction::Logic::Expression::Entry::From
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
         * @enum Nebulite::Interaction::Logic::Expression::Entry::CastType
         * @brief Represents the type of cast to apply to an expression entry.
         */
        enum CastType {
            none,       // No cast -> using pure string
            to_int,     // Cast to integer
            to_double   // Cast to double
        } cast = CastType::none; // Default to none

        /**
         * @brief If true, indicates leading zero formatting (e.g., 03.2f).
         */
        bool leadingZero = false;

        /**
         * @brief If positive, indicates the alignment size of the value.
         */
        int alignment = -1;

        /**
         * @brief If positive, indicates the precision size of the value.
         */
        int precision = -1;

        /**
         * @brief Holds the string representation of the entry.
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
         * @brief Holds the context-stripped key of the entry, if it's of type variable.
         */
        std::string key;

        /**
         * @brief Pointer to the tinyexpr representation of the expression.
         */
        te_expr* expression = nullptr;
    };

    /**
     * @brief Holds all virtual double entries for the self context.
     */
    std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>> virtualDoubles_self;

    /**
     * @brief Holds all virtual double entries for the other context.
     */
    std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>> virtualDoubles_other;

    /**
     * @brief Holds all virtual double entries for the global context.
     */
    std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>> virtualDoubles_global;

    /**
     * @brief Holds all virtual double entries for the resource context.
     */
    std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>> virtualDoubles_resource;

    /**
     * @brief A collection of custom functions for TinyExpr
     * 
     * Make sure to register all functions with TinyExpr in Nebulite::Interaction::Logic::Expression::reset
     */
    class expr_custom{
    public:
        // Logical comparison functions
        static double gt(double a, double b) {return a > b;}
        static double lt(double a, double b) {return a < b;}
        static double geq(double a, double b) {return a >= b;}
        static double leq(double a, double b) {return a <= b;}
        static double eq(double a, double b){return a == b;}
        static double neq(double a, double b){return a != b;}

        // Logical gate functions
        static double logical_not(double a){return !a;}

        static double logical_and(double a, double b){return a && b;}
        static double logical_or(double a, double b){return a || b;}
        static double logical_xor(double a, double b){return (a || b) && !(a && b);}

        static double logical_nand(double a, double b){return !(a && b);}
        static double logical_nor(double a, double b){return !(a || b);}
        static double logical_xnor(double a, double b){return !( (a || b) && !(a && b) );}

        // Other logical functions
        static double to_bipolar(double a){return a != 0 ? 1 : -1;}

        // Mapping functions
        static double map(double value, double in_min, double in_max, double out_min, double out_max) {
            if(in_max - in_min == 0) return out_min; // Prevent division by zero
            return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        }
        static double constrain(double value, double min, double max) {
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
     * @brief updates all internal caches of a virtual double
     * 
     * @param vec The vector of virtual double entries to update
     * @param link The JSON document to update the caches with
     */
    void updateCacheReference(std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>>* vec, Nebulite::Utility::JSON* link);

    /**
     * @brief Resets the expression to its initial state.
     * 
     * - Clears all entries
     * - Clears all variables and re-registers standard functions
     * - Clears all virtual double entries
     */
    void reset();

    /**
     * @brief Holds all parsed entries from the expression.
     */
    std::vector<Entry> entries;

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
    std::vector<te_variable> variables;   // Variables for TinyExpr evaluation

    /**
     * @brief Reference to the resource context
     */
    Nebulite::Utility::DocumentCache* documentCache = nullptr;

    //------------------------------------------
    // Helper functions

    /**
     * @brief Compiles an entry, if its of type Expression
     * 
     * @param entry The entry to potentially compile
     */
    void compileIfExpression(Entry& entry);

    /**
     * @brief Registers a variable with the given name and key in the context of the entry.
     * 
     * Makes sure to only register variables that are not already registered.
     * 
     * @param te_name The name of the variable as used in TinyExpr.
     * @param key The key in the JSON document that the variable refers to.
     * @param context The context from which the variable is being registered.
     */
    void registerVariable(std::string te_name, std::string key, Entry::From context);

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
    static std::string stripContext(const std::string& key);

    /**
     * @brief Gets the context from a key before it's stripped
     *
     * If the key doesnt start with `self.`, `other.`, or `global.`, it is considered a resource variable.
     * 
     * @param key The key to get the context from.
     * 
     * @return The context of the key.
     */
    static Entry::From getContext(const std::string& key);

    /**
     * @brief Parses the given expression into a series of entries.
     * 
     * @param expr The expression string to parse.
     * @param entries The vector to populate with the parsed entries.
     */
    void parseIntoEntries(const std::string& expr, std::vector<Entry>& entries);

    /**
     * @brief Reads the formatter string from a string and parses it intro the entry.
     * 
     * @param entry The entry to populate with the parsed formatter.
     * @param formatter The formatter string to parse.
     */
    static void readFormatter(Entry* entry, const std::string& formatter);

    /**
     * @brief Used to parse a string token of type "eval" into an entry.
     * 
     * - Parses the token on the assumption that it is of type "eval".
     * 
     * - Populates the current entry with the parsed information.
     * 
     * - Pushes the current entry onto the entries vector.
     * 
     * @param token The token to parse.
     * @param currentEntry The current entry to populate.
     * @param entries The vector to push the current entry onto.
     */
    void parseTokenTypeEval(std::string& token, Entry& currentEntry, std::vector<Entry>& entries);

    /**
     * @brief Used to parse a string token of type "text" into an entry.
     * 
     * - Parses the token on the assumption that it is of type "text".
     * 
     * - Populates the current entry with the parsed information.
     * 
     * - Pushes the current entry onto the entries vector.
     * 
     * @param token The token to parse.
     * @param currentEntry The current entry to populate.
     * @param entries The vector to push the current entry onto.
     */
    void parseTokenTypeText(std::string& token, Entry& currentEntry, std::vector<Entry>& entries);

    /**
     * @brief Prints a compilation error message to cerr
     * 
     * Includes tips for fixing the error.
     */
    void printCompileError(const Entry& entry, int& error);
};
} // namespace Logic
} // namespace Interaction
} // namespace Nebulite