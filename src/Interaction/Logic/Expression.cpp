//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "Data/Document/JsonScopeBase.hpp"
#include "Interaction/Logic/ExpressionPrimitives.hpp"
#include "Interaction/Logic/VirtualDouble.hpp"
#include "Interaction/Logic/Expression.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

//------------------------------------------
// Private:

Expression::~Expression() {
    // reset all data
    reset();

    // Remove variables
    te_variables.clear();

    // Clear all expressions
    for (auto const& component : components) {
        if (component->expression != nullptr) {
            te_free(component->expression);
            component->expression = nullptr;
        }
    }
}

void Expression::reset() {
    // Clear existing data
    components.clear();
    te_variables.clear();
    fullExpression.clear();
    components.clear();

    // Clear all variable names
    te_names.clear();

    // Clear all vds
    virtualDoubles.stable.self.clear();
    virtualDoubles.stable.other.clear();
    virtualDoubles.stable.global.clear();

    virtualDoubles.unstable.self.clear();
    virtualDoubles.unstable.other.clear();
    virtualDoubles.unstable.global.clear();
    virtualDoubles.unstable.resource.clear();

    //------------------------------------------
    // Register built-in functions
    ExpressionPrimitives::registerExpressions(te_variables);
}

std::string Expression::stripContext(std::string const& key) {
    constexpr std::array<std::string_view, 3> prefixes = {"self.", "other.", "global."};
    auto const it = std::ranges::find_if(prefixes, [&](std::string_view const p) {
        return key.size() >= p.size() && std::equal(p.begin(), p.end(), key.begin());
    });
    if (it != prefixes.end()) {
        return key.substr(it->size());
    }
    return key;
}

Expression::Component::ContextType Expression::getContextType(std::string const& key) {
    if (key.empty() || key.starts_with("|")) {
        return Component::ContextType::None;
    }
    if (key.starts_with("self.")) {
        return Component::ContextType::self;
    }
    if (key.starts_with("other.")) {
        return Component::ContextType::other;
    }
    if (key.starts_with("global.")) {
        return Component::ContextType::global;
    }
    return Component::ContextType::resource;
}

void Expression::compileIfExpression(std::shared_ptr<Component> const& component) const {
    if (component->type == Component::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        component->expression = te_compile(component->str.c_str(), te_variables.data(), static_cast<int>(te_variables.size()), &error);
        if (error) {
            printCompileError(component, error);

            // Resetting expression to nan, as explained in error print:
            // using nan directly is not supported.
            // 0/0 directly yields -nan, so we use abs(0/0)
            te_free(component->expression);
            component->expression = te_compile("abs(0/0)", te_variables.data(), static_cast<int>(te_variables.size()), &error);
        }
    }
}

namespace {
/**
 * @brief Checks if the expression can be returned as a double pointer.
 *        Meaning no multi-resolve or transformations are used.
 * @param key The key to check.
 * @return true if the expression can be returned as a double pointer, false otherwise.
 */
bool isAvailableAsDoublePtr(std::string const& key) {
    return key.find('{') == std::string::npos
        && key.find('}') == std::string::npos
        && key.find('|') == std::string::npos;
}
} // anonymous namespace

void Expression::registerVariable(std::string te_name, std::string const& key, Component::ContextType const& contextType) {
    // Check if variable exists in variables vector:
    bool const found = std::ranges::any_of(te_variables, [&](auto const& te_var) {
        if (te_var.name == te_name) {
            return true;
        }
        return false;
    });

    if (!found) {
        // Initialize with reference to document and cache register
        auto const vd = std::make_shared<VirtualDouble>(key);

        // Register cache based on context
        switch (contextType) {
        case Component::ContextType::self:
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.stable.self.push_back(vd);
            } else {
                virtualDoubles.unstable.self.push_back(vd);
            }
            break;
        case Component::ContextType::other:
            // Type other is always non-remanent, as other document reference can change
            // However, we need to distinguish between stable and unstable double pointers
            // Meaning the ones we can get from an ordered list, and the ones we need to resolve each time
            // (e.g. with multi-resolve or transformations)
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.stable.other.push_back(vd);
            } else {
                virtualDoubles.unstable.other.push_back(vd);
            }
            break;
        case Component::ContextType::global:
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.stable.global.push_back(vd);
            } else {
                virtualDoubles.unstable.global.push_back(vd);
            }
            break;
        case Component::ContextType::resource:
            virtualDoubles.unstable.resource.push_back(vd);
            break;
        case Component::ContextType::None:
            // Use an empty document
            virtualDoubles.unstable.none.push_back(vd);
            break;
        default:
            // Should not happen
            Error::println(__FUNCTION__, ": Tried to register variable with no known context!");
            std::unreachable();
        }

        // Store variable name for tinyexpr
        auto const te_name_ptr = std::make_shared<std::string>(te_name);
        te_names.push_back(te_name_ptr);

        // Push back into variable components
        te_variables.push_back({
            te_names.back()->c_str(),
            vd->ptr(),
            TE_VARIABLE,
            nullptr
        });
    }
}

void Expression::parseIntoComponents(std::string const& expr) {
    // First, we must split the expression into tokens
    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    std::vector<std::string> const tokensPhase1 = Utility::StringHandler::split(expr, '$', true);
    std::vector<std::string> tokens;

    // Now we need to split on same depth
    for (auto const& token : tokensPhase1) {
        // If the first token starts with '$', it means the string started with '$'
        // If not, the first token is text before the first '$'
        if (token.starts_with('$')) {
            // Remove everything until a '('
            // This part represents the '$' + formatter
            // Cannot be used, as splitOnSameDepth expects the first character to be the opening parenthesis
            std::string start = token.substr(0, token.find('('));
            std::string tokenWithoutStart = token.substr(start.length()); // Remove the leading '$'

            // Split on same depth
            std::vector<std::string> subTokens = Utility::StringHandler::splitOnSameDepth(tokenWithoutStart, '(');

            // Add back the '$' + formatter to first subToken
            if (!subTokens.empty()) {
                subTokens[0] = start + subTokens[0];
            }

            // Add all subtokens to the actual list of tokens
            std::ranges::copy(subTokens, std::back_inserter(tokens));
        } else {
            // If it doesn't start with a '$', it's a text token / potentially with variables inside
            // Just add the text token
            tokens.push_back(token);
        }
    }

    // Now we have a correct list of tokens. Either:
    // - evaluation
    // - text
    // Now we parse all tokens into a proper component, which further splits and categorizes them:
    // - general type: {variable, eval, text}
    // - cast type
    // - formatting
    // - splitting all text-variable mixes
    // - variable info (from what document, what the key is)
    for (auto const& token : tokens) {
        if (!token.empty()) {
            if (token.starts_with('$')) {
                parseTokenTypeEval(token);
            } else {
                parseTokenTypeText(token);
            }
        }
    }
}

void Expression::readFormatter(std::shared_ptr<Component> const& component, std::string const& formatter) {
    // Check formatter. Integer cast should not include precision. Is ignored later on in casting but acceptable as input
    // Examples:
    // $i     : leadingZero = false , alignment = -1 , precision = -1
    // $04i   : leadingZero = true  , alignment =  4 , precision = -1
    // $03.5i : leadingZero = true  , alignment =  3 , precision =  5

    if (formatter.empty()) {
        return;
    }

    // Read leading zero
    if (formatter.starts_with("0")) {
        component->formatter.leadingZero = true;
    }
    if (formatter.size() > 1) {
        size_t const dotPos = formatter.find('.');
        // Read alignment
        if (dotPos == 0) {
            component->formatter.alignment = 0;
        } else {
            component->formatter.alignment = std::stoi(formatter.substr(0, dotPos));
        }
        // Read precision
        if (dotPos != std::string::npos) {
            component->formatter.precision = std::stoi(formatter.substr(dotPos + 1));
        }
    }
}

void Expression::parseTokenTypeEval(std::string const& token) {
    // $[leading zero][alignment][.][precision]<type:f,i>
    // - bool leading zero   : on/off
    // - int alignment       : <0 means no formatting
    // - int precision       : <0 means no formatting
    // - CastType::none is then used to determine if we can simply use the double return from tinyexpr

    // 1.) find next '(' and split into formatter and token
    // Examples:
    // input        formatter       expression
    // $(1+1)       ""              "(1+1)"
    // $f(1.23)     "f"             "(1.23)"
    // $i(42)       "i"             "(42)"
    // $4.2f(2/3)   "4.2f"          "(2/3)"
    auto const currentComponent = std::make_shared<Component>();

    size_t const pos = token.find('(');
    std::string const formatter = token.substr(1, pos - 1); // Remove leading $
    std::string const expression = token.substr(pos);

    // Check cast type in formatter:
    if (formatter.ends_with("i")) {
        readFormatter(currentComponent, formatter);
        currentComponent->cast = Component::CastType::to_int;
    } else if (formatter.ends_with("f")) {
        readFormatter(currentComponent, formatter);
        currentComponent->cast = Component::CastType::to_double;
    } else {
        currentComponent->cast = Component::CastType::none;
    }

    // Register internal variables
    // And build equivalent expression using new variable names
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepth(expression, '{')) {
        if (subToken.starts_with('{')) {
            std::string const te_name = varNameGen.getUniqueName(subToken);
            std::string key = subToken.substr(1, subToken.length() - 2);
            Component::ContextType contextType = getContextType(key);
            key = stripContext(key);
            registerVariable(te_name, key, contextType);
            currentComponent->str += te_name;
        } else {
            currentComponent->str += subToken;
        }
    }

    // Write component data
    currentComponent->type = Component::Type::eval;
    currentComponent->contextType = Component::ContextType::None; // None, since this is an eval expression
    currentComponent->key = ""; // No key for eval expressions

    // Add to components
    components.push_back(currentComponent);
}

void Expression::parseTokenTypeText(std::string const& token) {
    // Current token is Text
    // Perhaps mixed with variables...
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepth(token, '{')) {
        auto currentComponent = std::make_shared<Component>();

        // Token is type variable
        // All variables starting with "{!" are considered text
        if (subToken.starts_with('{') && !subToken.starts_with("{!")) {
            // 1.) remove {}
            // We keep all other potential {} inside the variable name for later MultiResolve
            std::string inner;

            inner = subToken.substr(1, subToken.length() - 2);
            // 2.) determine context
            currentComponent->type = Component::Type::variable;
            currentComponent->str = inner;
            currentComponent->contextType = getContextType(inner);
            currentComponent->key = stripContext(inner);
        }
        // Token is type text
        else {
            // Determine context
            currentComponent->type = Component::Type::text;
            currentComponent->str = subToken;
            currentComponent->contextType = Component::ContextType::None;
            currentComponent->key = ""; // No key for text expressions
        }
        // Add to components
        components.push_back(currentComponent);
    }
}

void Expression::printCompileError(std::shared_ptr<Component> const& component, int const& error) const {
    std::string offendingChar;
    if (error <= 0 || static_cast<size_t>(error) > component->str.size()) {
        offendingChar = "N/A (error position out of bounds)";
    } else {
        offendingChar = std::string(1, component->str[static_cast<size_t>(error) - 1]);
    }
    std::stringstream ss;
    ss << "-----------------------------------------------------------------" << "\n";
    ss << "Error compiling expression: '" << component->str << "' At position: " << std::to_string(error) << ", offending character: " << offendingChar << "\n";
    ss << "You might see this message multiple times due to expression parallelization." << "\n";
    ss << "\n";
    ss << "If you only see the start of your expression, make sure to encompass your expression in quotes" << "\n";
    ss << "Some functions assume that the expression is inside, e.g. argv[1]." << "\n";
    ss << "Example: " << "\n";
    ss << "if $(1+1)     echo here! # works" << "\n";
    ss << "if $(1 + 1)   echo here! # doesnt work!" << "\n";
    ss << "if '$(1 + 1)' echo here! # works" << "\n";
    ss << "\n";
    ss << "Registered functions and variables:\n";
    for (auto const& var : te_variables) {
        ss << "\t'" << var.name << "'\n";
    }
    ss << "\n";
    ss << "Resetting expression to always yield 'nan'" << "\n";
    ss << "\n";
    ss << "\n";

    // Send whole message to cerr at once, to avoid interleaving with other messages
    Error::println(ss.str());
}

//------------------------------------------
// Public:

Expression::Expression(std::string const& expr){
    _isReturnableAsDouble = false;
    _isAlwaysTrue = false;
    reset();
    parse(expr);
}

void Expression::parse(std::string const& expr) {
    reset();
    fullExpression = expr;
    parseIntoComponents(expr);
    for (auto& component : components) {
        compileIfExpression(component);
    }
    _isReturnableAsDouble = recalculateIsReturnableAsDouble();
    _isAlwaysTrue = recalculateIsAlwaysTrue();

    // Reset variable name generator, data is only needed during parsing
    varNameGen.clear();
}

//------------------------------------------
// Static one-time evaluation

// With context

std::string Expression::eval(std::string const& input, ContextScopeBase const& context) {
    Expression const expr(input);
    return expr.eval(context);
}

double Expression::evalAsDouble(std::string const& input, ContextScopeBase const& context) {
    Expression const expr(input);
    return expr.evalAsDouble(context);
}

bool Expression::evalAsBool(std::string const& input, ContextScopeBase const& context) {
    Expression const expr(input);
    return expr.evalAsBool(context);
}

Data::JSON Expression::evalAsJson(std::string const& input, ContextScopeBase const& context) {
    Expression const expr(input);
    return expr.evalAsJson(context);
}

// Global-only as context

std::string Expression::eval(std::string const& input) {
    ContextScopeBase const context{emptyDoc(), emptyDoc(), globalDoc()};
    return eval(input, context);
}

double Expression::evalAsDouble(std::string const& input) {
    ContextScopeBase const context{emptyDoc(), emptyDoc(), globalDoc()};
    return evalAsDouble(input, context);
}

bool Expression::evalAsBool(std::string const& input) {
    ContextScopeBase const context{emptyDoc(), emptyDoc(), globalDoc()};
    return evalAsBool(input, context);
}

Data::JSON Expression::evalAsJson(std::string const& input) {
    ContextScopeBase const context{emptyDoc(), emptyDoc(), globalDoc()};
    return evalAsJson(input, context);
}

//------------------------------------------
// Private helper functions

void Expression::updateCaches(ContextScopeBase const& context) const {
    if (!firstEvaluationContext.self) {
        firstEvaluationContext.self = &context.self;
        for (auto const& vde : virtualDoubles.stable.self) {
            vde->linkExternalCache(context.self);
        }

        firstEvaluationContext.other = &context.other;
        for (auto const& vde : virtualDoubles.stable.other) {
            vde->linkExternalCache(context.other);
        }

        firstEvaluationContext.global = &context.global;
        for (auto const& vde : virtualDoubles.stable.global) {
            vde->linkExternalCache(context.global);
        }
    }

    //===================================================
    // Stable values:
    // either copy from first context via pointers (fast),
    // or copy from the current context via json->get (slow)
    //===================================================

    // self
    if (&context.self == firstEvaluationContext.self) {
        for (auto const& vde : virtualDoubles.stable.self) {
            vde->copyExternalCache();
        }
    }
    else {
        for (auto const& vde : virtualDoubles.stable.self) {
            vde->copyFromJson(context.self);
        }
    }

    // other
    if (&context.other == firstEvaluationContext.other) {
        for (auto const& vde : virtualDoubles.stable.other) {
            vde->copyExternalCache();
        }
    }
    else {
        for (auto const& vde : virtualDoubles.stable.other) {
            vde->copyFromJson(context.other);
        }
    }

    // global
    if (&context.global == firstEvaluationContext.global) {
        for (auto const& vde : virtualDoubles.stable.global) {
            vde->copyExternalCache();
        }
    }
    else {
        for (auto const& vde : virtualDoubles.stable.global) {
            vde->copyFromJson(context.global);
        }
    }

    //===================================================
    // Unstable values:
    // evaluate key and fetch value from json
    //===================================================

    for (auto const& vde : virtualDoubles.unstable.self) {
        auto const key = Data::ScopedKey(eval(vde->getKey(), context));
        vde->setDirect(context.self.get<double>(key, 0.0));
    }
    for (auto const& vde : virtualDoubles.unstable.other) {
        auto const key = Data::ScopedKey(eval(vde->getKey(), context));
        vde->setDirect(context.other.get<double>(key, 0.0));
    }
    for (auto const& vde : virtualDoubles.unstable.global) {
        auto const key = Data::ScopedKey(eval(vde->getKey(), context));
        vde->setDirect(context.global.get<double>(key, 0.0));
    }
    for (auto const& vde : virtualDoubles.unstable.resource) {
        // Since resource documents may be unloaded at any time, we must always fetch the value instead of using stable double pointers
        auto const key = eval(vde->getKey(), context);
        vde->setDirect(Global::instance().getDocCache().get<double>(key, 0.0));
    }
    for (auto const& vde : virtualDoubles.unstable.none) {
        auto const key = Data::ScopedKey(eval(vde->getKey(), context));
        vde->setDirect(emptyDoc().get<double>(key, 0.0));
    }
}


//------------------------------------------
// Other Static helpers

std::string Expression::removeOuterAntiEvalWrapper(std::string const& expression) {
    auto expressionParts = Utility::StringHandler::splitOnSameDepth(expression, '{');
    for (auto& expressionPart : expressionParts) {
        // Any outer expression part with an anti-evaluation wrapper {! ... } should have it replaced with { ... }
        // as we want to evaluate the expression inside as normal
        if (expressionPart.starts_with("{!")) {
            expressionPart = "{" + expressionPart.substr(2);
        }
    }

    // Recombine to form the full expression
    return std::accumulate(expressionParts.begin(), expressionParts.end(), std::string{});
}

//------------------------------------------
// Recalculation helpers:

bool Expression::recalculateIsReturnableAsDouble() const {
    return components.size() == 1
           && components[0]->type == Component::Type::eval
           && components[0]->cast == Component::CastType::none;
}

bool Expression::recalculateIsAlwaysTrue() const {
    return fullExpression == "1" || fullExpression == "$(1)";
}

//------------------------------------------
// Actual expression evaluation

std::string Expression::eval(ContextScopeBase const& context, uint16_t const& max_recursion_depth) const {
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(context);

    //------------------------------------------
    // Evaluate expression
    // Concatenate results of each component
    // TODO: Best to have both result and token be part of the class?
    //       Set to empty should not be needed, as any previous data is overwritten?
    std::string result;
    for (auto const& component : components) {
        std::string token;
        switch (component->type) {
            //------------------------------------------
        case Component::Type::variable:
            if (!component->handleComponentTypeVariable(token, context, max_recursion_depth)) {
                token = "null";
            }
            break;
            //------------------------------------------
        case Component::Type::eval:
            component->handleComponentTypeEval(token);
            break;
            //------------------------------------------
        case Component::Type::text:
            token = component->str;
            break;
            //------------------------------------------
        default:
            break;
        }
        result += token;
    }
    return result;
}

double Expression::evalAsDouble(ContextScopeBase const& context) const {
    updateCaches(context);
    return te_eval(components[0]->expression);
}

bool Expression::evalAsBool(ContextScopeBase const& context) const {
    double const result = evalAsDouble(context);
    return std::fabs(result) > DBL_EPSILON;
}

Data::JSON Expression::evalAsJson(ContextScopeBase const& context, uint16_t const& max_recursion_depth) const {
    if (components.size() == 1 && components[0]->type != Component::Type::text) {
        if (components[0]->type == Component::Type::eval) {
            Data::JSON jsonResult;
            jsonResult.set<double>("", evalAsDouble(context));
            return jsonResult;
        }
        if (components[0]->type == Component::Type::variable) {
            Data::JSON jsonResult;
            components[0]->handleComponentTypeVariable(jsonResult, context, max_recursion_depth);
            return jsonResult;
        }
    }
    Data::JSON jsonResult;
    jsonResult.set<std::string>("", eval(context, max_recursion_depth));
    return jsonResult;
}

//------------------------------------------
// Static document access

Data::JsonScopeBase& Expression::emptyDoc() {
    thread_local Data::JsonScopeBase emptyDoc;
    return emptyDoc;
}

Data::JsonScopeBase& Expression::globalDoc() {
    static auto accessToken = ScopeAccessor::Full();
    static auto& globalDoc = Global::shareScopeBase(accessToken, "");
    return globalDoc;
}

}   // namespace Nebulite::Interaction::Logic
