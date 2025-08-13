#pragma once

#include "VirtualDouble.h"
#include "string"
#include "DocumentCache.h"
#include "tinyexpr.h"
#include <memory>
#include <deque>


// Set to use external cache, meaning double-values from inside expressions use the JSON storage directly
#define use_external_cache 1

/*
Instead of storing strings, we store a full expression that's able to evaluate relevant parts:

'1 + 1 = $i(1+1)' -> '1 + 1 = 2'
*/



namespace Nebulite {



class InvokeExpression {
public:
    InvokeExpression();

    void parse(const std::string& expr, Nebulite::DocumentCache& documentCache, Nebulite::JSON* self, Nebulite::JSON* global);
    std::string eval(Nebulite::JSON* current_other);
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
            none, to_int, to_float
        } cast = CastType::none; // Default to none

        std::string str;
        std::string key;

        // If of type te_expr, this will be initialized:
        te_expr* expression = nullptr;
    };
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

    void clear() {
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

    std::vector<Entry> entries;
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

