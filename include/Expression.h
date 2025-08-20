/**
 * @file Expression.h
 * 
 * This file contains the definition of the Expression class, which is responsible for parsing and evaluating expressions within the Nebulite engine.
 */

#pragma once

#include "VirtualDouble.h"
#include "string"
#include "DocumentCache.h"
#include "tinyexpr.h"
#include <memory>
#include <deque>


// Set to use external cache, meaning double-values from inside expressions use the JSON storage directly
#define use_external_cache 1


namespace Nebulite {


/**
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

    /**
     * @brief Parses a given expression string with a constant reference to the document cache and the self and global JSON objects.
     */
    void parse(const std::string& expr, Nebulite::DocumentCache& documentCache, Nebulite::JSON* self, Nebulite::JSON* global);

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
    double evalAsDouble(Nebulite::JSON* current_other);
    
    /**
     * @brief Evaluates the expression as a string.
     * 
     * @param current_other The JSON object `other` to evaluate against.
     * @return The evaluated string value.
     */
    std::string eval(Nebulite::JSON* current_other);

    /**
     * @brief Gets the full expression string that was parsed.
     * 
     * @return The full expression string.
     */
    std::string getFullExpression(){return fullExpression;};

private:
    // Links to self, global, doccache stay the same
    Nebulite::JSON* self = nullptr;
    Nebulite::JSON* global = nullptr;
    Nebulite::DocumentCache* globalCache = nullptr;

    struct Entry {
        enum Type {
            variable,   // inside $<cast>(...), Starts with self, other, global or a dot for link
            eval,       // inside $<cast>(...), all other cases
            text        // outside of a $<cast>(...)
        } type = Type::text;

        // For variable
        enum From {
            self, other, global, resource, None
        } from = From::None; // Default to None

        // Cast Type
        enum CastType {
            none, to_int, to_double
        } cast = CastType::none; // Default to none

        // Formatting
        bool leadingZero = false;
        int alignment = -1;
        int precision = -1;

        // Holds internal values derived from full expression
        std::string str;
        std::string key;

        // If of type te_expr, this will be initialized:
        te_expr* expression = nullptr;
    };

    /**
     * @brief Holds all virtual double entries.
     * 
     * A virtual double entry represents a double value needed within a tinyexpr evaluation.
     * We use these entries to bridge the gap between the JSON document structure and the expression evaluation.
     * On evaluation, we update all double pointers from the JSON document to the tinyexpr context.
     * 
     * Depending on the document type, we either register the values inside the vd_entry or in the json document
     * Both remanent and non-remanent types use the vd_entry for variable management.
     */
    struct vd_entry {
        std::shared_ptr<Nebulite::VirtualDouble> virtualDouble;
        Entry::From from;
        std::string key;
        std::string te_name;
    };

    std::vector<std::shared_ptr<vd_entry>> virtualDoubles_self;
    std::vector<std::shared_ptr<vd_entry>> virtualDoubles_other;
    std::vector<std::shared_ptr<vd_entry>> virtualDoubles_global;
    std::vector<std::shared_ptr<vd_entry>> virtualDoubles_resource;

    void update_vds(std::vector<std::shared_ptr<vd_entry>>* vec, Nebulite::JSON* link){
        for(auto& vde : *vec) {
            vde->virtualDouble->updateCache(link);
        }
    }

    void reset() {
        documentCache = nullptr;
        self = nullptr;
        global = nullptr;

        // Clear existing data
        entries.clear();
        variables.clear();
        fullExpression.clear();
        entries.clear();

        // Clear vds
        virtualDoubles_self.clear();
        virtualDoubles_other.clear();
        virtualDoubles_global.clear();
        virtualDoubles_resource.clear();

        // Register built-in functions
        te_variable gt_var =  {"gt",    (void*)expr_custom::gt,             TE_FUNCTION2};
        variables.push_back(gt_var);
        te_variable lt_var =  {"lt",    (void*)expr_custom::lt,             TE_FUNCTION2};
        variables.push_back(lt_var);
        te_variable geq_var = {"geq",   (void*)expr_custom::geq,            TE_FUNCTION2};
        variables.push_back(geq_var);
        te_variable leq_var = {"leq",   (void*)expr_custom::leq,            TE_FUNCTION2};
        variables.push_back(leq_var);
        te_variable eq_var =  {"eq",    (void*)expr_custom::eq,             TE_FUNCTION2};
        variables.push_back(eq_var);
        te_variable neq_var = {"neq",   (void*)expr_custom::neq,            TE_FUNCTION2};
        variables.push_back(neq_var);
        te_variable and_var = {"and",   (void*)expr_custom::logical_and,    TE_FUNCTION2};
        variables.push_back(and_var);
        te_variable or_var =  {"or",    (void*)expr_custom::logical_or,     TE_FUNCTION2};
        variables.push_back(or_var);
        te_variable not_var = {"not",   (void*)expr_custom::logical_not,    TE_FUNCTION1};
        variables.push_back(not_var);
        te_variable sgn_var = {"sgn",   (void*)expr_custom::sgn,            TE_FUNCTION1};
        variables.push_back(sgn_var);
    }

    /**
     * @brief Holds all parsed entries from the expression.
     */
    std::vector<Entry> entries;

    /**
     * @brief Holds the full expression as a string.
     */
    std::string fullExpression;

    std::vector<te_variable> variables;   // Variables for TinyExpr evaluation

    // Cache is passed to all virtual doubles as well
    Nebulite::DocumentCache* documentCache = nullptr;

    // Helper functions
    std::string stripContext(const std::string& key) {
        if (key.starts_with("self.")) {
            return key.substr(5);
        } else if (key.starts_with("other.")) {
            return key.substr(6);
        } else if (key.starts_with("global.")) {
            return key.substr(7);
        } else {
            return key;
        }
    }
    Entry::From getContext(const std::string& key) {
        if (key.starts_with("self.")) {
            return Entry::From::self;
        } else if (key.starts_with("other.")) {
            return Entry::From::other;
        } else if (key.starts_with("global.")) {
            return Entry::From::global;
        } else {
            return Entry::From::resource;
        }
    }

    void parseIntoEntries(const std::string& expr, std::vector<Entry>& entries);
    void compileIfExpression(Entry& entry);
    void registerVariable(std::string str, std::string key, Entry::From context);
    void readFormatter(Entry* entry, const std::string& formatter);

    void parseTokenTypeEval(std::string& token, Entry& currentEntry, std::vector<Entry>& entries);
    void parseTokenTypeText(std::string& token, Entry& currentEntry, std::vector<Entry>& entries);

    void printCompileError(const Entry& entry, int& error);

    // Custom TinyExpr functions
    class expr_custom{
    public:
        static double gt(double a, double b) {return a > b;}
        static double lt(double a, double b) {return a < b;}
        static double geq(double a, double b) {return a >= b;}
        static double leq(double a, double b) {return a <= b;}
        
        static double eq(double a, double b){return a == b;}
        static double neq(double a, double b){return a != b;}

        static double logical_and(double a, double b){return a && b;}
        static double logical_or(double a, double b){return a || b;}
        static double logical_not(double a){return !a;}

        static double sgn(double a){return std::copysign(1.0, a);}
    };

    // Storing info about the expression's returnability
    bool _isReturnableAsDouble;
};
} // namespace Nebulite

