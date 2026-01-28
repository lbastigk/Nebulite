//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "Data/RollingId.hpp"
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
    virtualDoubles.remanent.self.clear();
    virtualDoubles.remanent.global.clear();
    virtualDoubles.nonRemanent.self.clear();
    virtualDoubles.nonRemanent.other.clear();
    virtualDoubles.nonRemanent.global.clear();
    virtualDoubles.nonRemanent.resource.clear();

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

Expression::Component::ContextType Expression::getContext(std::string const& key) {
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

void Expression::registerVariable(std::string te_name, std::string const& key, Component::ContextType const& context) {
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
        switch (context) {
        case Component::ContextType::self:
            if (isAvailableAsDoublePtr(key)) {
                vd->setUpExternalCache(self);
                virtualDoubles.remanent.self.push_back(vd);
            } else {
                vd->setUpExternalCache(self);
                virtualDoubles.nonRemanent.self.push_back(vd);
            }
            break;
        case Component::ContextType::other:
            // Type other is always non-remanent, as other document reference can change
            // However, we need to distinguish between stable and unstable double pointers
            // Meaning the ones we can get from an ordered list, and the ones we need to resolve each time
            // (e.g. with multi-resolve or transformations)
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.nonRemanent.other.push_back(vd);
            } else {
                virtualDoubles.nonRemanent.otherUnStable.push_back(vd);
            }
            break;
        case Component::ContextType::global:
            if (isAvailableAsDoublePtr(key)) {
                vd->setUpExternalCache(Global::instance().domainScope.shareScopeBase(""));
                virtualDoubles.remanent.global.push_back(vd);
            } else {
                virtualDoubles.nonRemanent.global.push_back(vd);
            }
            break;
        case Component::ContextType::resource:
            // Type resource is always non-remanent, as document cache can change (?)
            // TODO: double check if remanent resource variables are possible
            virtualDoubles.nonRemanent.resource.push_back(vd);
            break;
        case Component::ContextType::None:
            // Use an empty document
            virtualDoubles.nonRemanent.none.push_back(vd);
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
            Component::ContextType context = getContext(key);
            key = stripContext(key);
            registerVariable(te_name, key, context);
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
        if (subToken.starts_with('{')) {
            // 1.) remove {}
            // We keep all other potential {} inside the variable name for later MultiResolve
            std::string inner;

            inner = subToken.substr(1, subToken.length() - 2);
            // 2.) determine context
            currentComponent->type = Component::Type::variable;
            currentComponent->str = inner;
            currentComponent->contextType = getContext(inner);
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

Expression::Expression(std::string const& expr, Data::JsonScopeBase& selfScope)
    : self(selfScope)
{
    _isReturnableAsDouble = false;
    _isAlwaysTrue = false;
    uniqueId = 0;
    reset();
    parse(expr);
}

void Expression::parse(std::string const& expr) {
    reset();
    fullExpression = expr;
    //uniqueId = Global::instance().getUniqueId(fullExpression, Core::GlobalSpace::UniqueIdType::expression);
    uniqueId = generateUniqueId(fullExpression);
    parseIntoComponents(expr);
    for (auto& component : components) {
        compileIfExpression(component);
    }
    // Reset variable name generator, data is only needed during parsing
    varNameGen.clear();
}

Data::JSON Expression::evalAsJson(Core::JsonScope& current_other, uint16_t const& max_recursion_depth) const {
    if (components.size() == 1 && components[0]->type != Component::Type::text) {
        if (components[0]->type == Component::Type::eval) {
            Data::JSON jsonResult;
            jsonResult.set<double>("", evalAsDouble(current_other));
            return jsonResult;
        }
        if (components[0]->type == Component::Type::variable) {
            Data::JSON jsonResult;
            components[0]->handleComponentTypeVariable(jsonResult, self, current_other, max_recursion_depth);
            return jsonResult;
        }
    }
    Data::JSON jsonResult;
    jsonResult.set<std::string>("", eval(current_other, max_recursion_depth));
    return jsonResult;
}

std::string Expression::eval(Core::JsonScope& current_other, uint16_t const& max_recursion_depth) const {
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

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
            if (!component->handleComponentTypeVariable(token, self, current_other, max_recursion_depth)) {
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

double Expression::evalAsDouble(Core::JsonScope& current_other) const {
    updateCaches(current_other);
    return te_eval(components[0]->expression);
}

void Expression::updateCaches(Core::JsonScope& reference) const {
    // Update self references that are non-remanent
    for (auto const& vde : virtualDoubles.nonRemanent.self) {
        // One-time handle of multi-resolve and transformations
        Expression const tempExpr(vde->getKey(), self);
        auto const evalResult = Data::ScopedKey(tempExpr.eval(reference));
        vde->setDirect(reference.get<double>(evalResult, 0.0));
    }

    // Updating context other: Values with stable double pointers
    // We can use a cached list of double pointers for this, speeding up access significantly
    // Document other stores a list of ordered double pointers for our expression
    // So we only have one query to get all pointers instead of querying each variable individually
    if (!virtualDoubles.nonRemanent.other.empty()) {
        auto const* listData = reference.getOrderedCacheListMap()->ensureOrderedCacheList(uniqueId, virtualDoubles.nonRemanent.other)->data();
        const size_t count = virtualDoubles.nonRemanent.other.size();
        for (size_t i = 0; i < count; ++i) {
            virtualDoubles.nonRemanent.other[i]->setDirect(*listData[i]);
        }
    }

    // Updating context other: Values without stable double pointers
    for (auto const& vde : virtualDoubles.nonRemanent.otherUnStable) {
        // One-time handle of multi-resolve and transformations
        Expression tempExpr(vde->getKey(), self);
        auto const evalResult = Data::ScopedKey(tempExpr.eval(reference));
        vde->setDirect(reference.get<double>(evalResult, 0.0));
    }

    // Update global references that are non-remanent
    for (auto const& vde : virtualDoubles.nonRemanent.global) {
        // One-time handle of multi-resolve and transformations
        Expression tempExpr(vde->getKey(), self);
        auto const evalResult = Data::ScopedKey(tempExpr.eval(reference));
        auto const val = Global::instance().domainScope.get<double>(evalResult, 0.0);
        vde->setDirect(val);
    }

    // Update resource references
    for (auto const& vde : virtualDoubles.nonRemanent.resource) {
        if (isAvailableAsDoublePtr(vde->getKey())) {
            vde->setUpInternalCache();
        } else {
            // One-time handle of multi-resolve and transformations
            Expression tempExpr(vde->getKey(), self);
            std::string const evalResult = tempExpr.eval(reference);
            vde->setDirect(Global::instance().getDocCache().get<double>(evalResult, 0.0));
        }
    }

    // Update none-context references
    for (auto const& vde : virtualDoubles.nonRemanent.none) {
        // One-time handle of multi-resolve and transformations
        Expression tempExpr(vde->getKey(), self);
        std::string const evalResult = tempExpr.eval(reference);
        // This requires an empty document that acts as a parsing mechanism for the transformations
        thread_local Core::JsonScope emptyDoc;
        vde->setDirect(emptyDoc.get<double>(Data::ScopedKey(evalResult).view(), 0.0));
    }
}

//------------------------------------------
// Static one-time evaluation

// With context

std::string Expression::eval(std::string const& input, ContextBase const& context) {
    Expression const expr(input, context.self.domainScope);
    return expr.eval(context.other.domainScope);
}

double Expression::evalAsDouble(std::string const& input, ContextBase const& context) {
    Expression const expr(input, context.self.domainScope);
    return expr.evalAsDouble(context.other.domainScope);
}

bool Expression::evalAsBool(std::string const& input, ContextBase const& context) {
    double const result = evalAsDouble(input, context);
    return std::fabs(result) > DBL_EPSILON;
}

Data::JSON Expression::evalAsJson(std::string const& input, ContextBase const& context) {
    Expression const expr(input, context.self.domainScope);
    return expr.evalAsJson(context.other.domainScope);
}

// Global-only as context

std::string Expression::eval(std::string const& input) {
    Core::JsonScope emptyDoc;
    ContextBase const context{emptyDoc, emptyDoc, Global::instance()};
    return eval(input, context);
}

double Expression::evalAsDouble(std::string const& input) {
    Core::JsonScope emptyDoc;
    ContextBase const context{emptyDoc, emptyDoc, Global::instance()};
    return evalAsDouble(input, context);
}

bool Expression::evalAsBool(std::string const& input) {
    Core::JsonScope emptyDoc;
    ContextBase const context{emptyDoc, emptyDoc, Global::instance()};
    return evalAsBool(input, context);
}

//------------------------------------------
// Other Static helpers

uint64_t Expression::generateUniqueId(std::string const& expression) {
    static Data::RollingId idGenerator;
    return idGenerator.getId(expression);
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

}   // namespace Nebulite::Interaction::Logic
