#pragma once

#include "VirtualDouble.h"
#include "string"
#include "DocumentCache.h"
#include "tinyexpr.h"
#include <memory>
#include <deque>

/*
Instead of storing strings, we store a full expression that's able to evaluate relevant parts:

'1 + 1 = $i(1+1)' -> '1 + 1 = 2'
*/



namespace Nebulite {



class InvokeExpression {
public:
    InvokeExpression();

    void parse(const std::string& expr, Nebulite::DocumentCache& documentCache);
    std::string eval(Nebulite::JSON* current_self, Nebulite::JSON* current_other, Nebulite::JSON* current_global);

    std::string getFullExpression(){return fullExpression;};

    void clear() {
        fullExpression.clear();
        entries.clear();
        parse("0", *documentCache);
    }

private:
    struct Entry {
        enum Type {
            variable,   // inside $<cast>(...), Starts with self, other, global or a dot for link
            eval,       // inside $<cast>(...), all other cases
            text        // outside of a $<cast>(...)
        } type = Type::text;

        // For variable
        enum From {
            self, other, global, resource
        } from = From::resource; // Default to resource

        // Cast Type
        enum CastType {
            none, to_int, to_float
        } cast = CastType::none; // Default to none

        // Depending on Entry Type, holds different value
        // $(global.myKey) -> myKey
        // $(./Resources/.../myFile.jsonc:myKey) -> ./Resources/.../myFile.jsonc:myKey
        std::string str;

        // If of type te_expr, this will be initialized:
        te_expr* expression = nullptr;
    };

    std::vector<Entry> entries;
    Nebulite::DocumentCache* globalCache;
    std::string fullExpression;

    std::vector<te_variable> variables;                     // Variables for TinyExpr evaluation
    std::vector<std::shared_ptr<Nebulite::VirtualDouble>> virtualDoubles;    // Virtual doubles for TinyExpr evaluation
    std::deque<std::string> variableNames;                                  // Persistent storage for variable names

    Nebulite::JSON* self = nullptr;
    Nebulite::JSON* other = nullptr;
    Nebulite::JSON* global = nullptr;
    Nebulite::DocumentCache* documentCache = nullptr;

    // Helper functions
    std::string modifyTextToTeConform(std::string str);
    void parseIntoEntries(const std::string& expr, std::vector<Entry>& entries);
    void setEntryContext(Entry& entry);
    void compileIfExpression(Entry& entry);
    void registerIfVariable(Entry& entry);
    

    void make_entry(Entry& currentEntry, std::vector<Entry>& entries);

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
};
}

