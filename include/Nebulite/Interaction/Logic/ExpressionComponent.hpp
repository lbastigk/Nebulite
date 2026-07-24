#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSIONCOMPONENT_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSIONCOMPONENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// External
#include <tinyexpr.h>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Formatter.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Logic {
class VariableNameGenerator;
} // namespace Nebulite::Interaction::Logic

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @struct Nebulite::Interaction::Logic::ExpressionComponent
 * @brief Represents a single component in an expression, such as a variable, evaluation, or text.
 * @details Holds information about a specific part of the expression,
 *          including its type, source, and any associated metadata.
 * @todo Since the entire parsing and evaluation is internal, we could wrap this in a variant to reduce memory usage
 */
class ExpressionComponent {
public:
    /**
     * @brief Destructor to clean up allocated resources.
     */
    ~ExpressionComponent() {
        te_free(expression);
    }

    // disable copying
    ExpressionComponent(ExpressionComponent const&) = delete;
    ExpressionComponent& operator=(ExpressionComponent const&) = delete;

    // enable moving
    ExpressionComponent(ExpressionComponent&& other) = default ;
    ExpressionComponent& operator=(ExpressionComponent&& other) = default ;

    void reset() {
        if (expression != nullptr) {
            te_free(expression);
            expression = nullptr;
        }
    }

    //------------------------------------------
    // Type

    /**
     * @enum Nebulite::Interaction::Logic::ExpressionComponent::Type
     * @brief Each component can be of type variable, eval or text that differ in how they are evaluated.
     */
    enum class Type : std::uint8_t {
        variable, // outside $<cast>(...), Starts with self, other, global or a dot for link, represents a variable reference, outside an evaluatable context
        eval, // inside a $<cast>(...), represents an evaluatable expression
        text // outside a $<cast>(...), not a variable reference, Represents a plain text string
    };

    //------------------------------------------
    // Component generation methods

    /**
     * @brief Parses an eval component from a given token, generating unique variable names and registering them as needed.
     * @param token The token representing the eval component to parse.
     * @param varNameGen The VariableNameGenerator instance used to generate unique variable names for the eval component.
     * @param registerVariableCallback A callback function to register variables found within the eval component. It takes the generated variable name, the original key, and the context type as parameters.
     * @return The parsed eval component.
     */
    static ExpressionComponent parseEval(std::string_view token, VariableNameGenerator& varNameGen, std::function<void(std::string_view, std::string_view, ContextDeriver::TargetType)> const& registerVariableCallback);

    /**
     * @brief Parses a text component from a given token.
     * @param token The token representing the text component to parse.
     * @return The parsed text component.
     */
    static ExpressionComponent parseText(std::string_view token);

    /**
     * @brief Parses a variable component from a given token, extracting the context and key information.
     * @param token The token representing the variable component to parse.
     * @return The parsed variable component.
     */
    static ExpressionComponent parseVariable(std::string_view token);

    //------------------------------------------
    // Compile

    /**
     * @brief Compiles a component, if its of type Expression
     * @param te_variables The vector of TinyExpr variables
     */
    int compile(std::vector<te_variable> const& te_variables);

    //------------------------------------------
    // Getter

    [[nodiscard]]  std::string const& getStringRepresentation() const ;

    //------------------------------------------
    // Returnability

    [[nodiscard]] bool isSimpleExpression() const noexcept ;

    [[nodiscard]] bool isSimpleExpressionWithIntCast() const noexcept ;

    [[nodiscard]] bool isReturnableAsString() const noexcept;

    //------------------------------------------
    // Evaluation

    /**
     * @brief Evaluates the component as a string.
     * @param result The string to store the evaluation result.
     * @param context The context to evaluate against.
     * @param recursionDepth The current recursion depth for nested evaluations.
     */
    void eval(std::string& result, ContextScope const& context, std::size_t recursionDepth) const ;

    /**
     * @brief Evaluates the component as a double.
     * @details Only valid if the component is returnable as double or int!
     * @tparam F The type of the function object to update caches.
     * @param cacheUpdater The function object to update caches.
     * @return The evaluation result as a double.
     */
    template<typename F>
    [[nodiscard]] double evalAsDouble(F cacheUpdater) const {
        cacheUpdater();
        return evalAsDoubleImpl();
    }

    /**
     * @brief Evaluates the component as a JSON object.
     * @tparam F The type of the function object to update caches.
     * @param context The context of the interaction
     * @param recursionDepth The current recursion depth for nested evaluations.
     * @param cacheUpdater The function object to update caches.
     * @return The evaluation result as a JSON object.
     */
    template<typename F>
    [[nodiscard]] Data::JSON evalAsJson(ContextScope const& context, std::size_t recursionDepth, F cacheUpdater) const {
        cacheUpdater();
        return evalAsJsonImpl(context, recursionDepth);
    }

private:
    Type type = Type::text;

    ContextDeriver::TargetType contextType = ContextDeriver::TargetType::none; // Default to none

    // Formatting options for this component, if applicable
    Formatter formatter;

    /**
     * @brief Holds the string representation of the component.
     *        Depending on context Either:
     *        - The Expression to evaluate, with formatting specifiers removed
     *        - The pure text
     *        - The variable key, with no context stripped
     */
    std::string stringRepresentation;

    /**
     * @brief Holds the context-stripped key of the component, if it's of type variable.
     */
    std::string key;

    /**
     * @brief The evaluation wait count. Used to delay the evaluation of a component.
     * @details For each evaluation, the count is reduced by 1. Only for component type Variable!
     */
    std::size_t evaluationWait = 0;

    /**
     * @brief Pointer to the tinyexpr representation of the expression.
     */
    te_expr* expression = nullptr;

    /**
     * @brief Default constructor for Component.
     */
    ExpressionComponent() = default;

    enum class KeyEvaluationInfo : std::uint8_t {
        maximumDepthReached, // Could not resolve due to maximum depth reached
        noNesting // No nested variables found
    };

    /**
     * @brief For variable component handling. Evaluates any inner expressions/variables within the component's key and returns the resulting key.
     * @details If the key, for example is nested: {global:{self:info.requiredKey}}, it turns into {global:evaluatedValueOfRequiredKey}
     *          and fetches that value from the global document.
     * @param context The context to evaluate against.
     * @param recursionDepth The current recursion depth for nested evaluations.
     * @return The evaluated string if successful, or std::nullopt if evaluation fails.
     */
    [[nodiscard]] std::expected<std::string, KeyEvaluationInfo> evaluateKey(ContextScope const& context, std::size_t recursionDepth) const ;

    [[nodiscard]] std::optional<std::pair<std::string, ContextDeriver::TargetType>> handleNesting(ContextScope const& context, std::size_t recursionDepth) const ;

    /**
     * @brief Handles the evaluation of a variable component as a string.
     * @details Takes care of proper conversion to string, with abbreviated representations for non-value types (arrays, objects and null).
     * @param token The string to populate with the evaluated value.
     * @param context The context to evaluate against.
     * @param recursionDepth The current recursion depth for nested evaluations.
     * @return True if the evaluation was successful, false otherwise.
     */
    bool evalComponentTypeVariable(std::string& token, ContextScope const& context, std::size_t recursionDepth) const ;

    /**
     * @brief Handles the evaluation of a variable component as a JSON value.
     * @details Populates the provided JSON object with the evaluated value, preserving its type.
     * @param token The JSON object to populate with the evaluated value.
     * @param context The context to evaluate against.
     * @param recursionDepth The current recursion depth for nested evaluations.
     * @return True if the evaluation was successful, false otherwise.
     */
    bool evalComponentTypeVariable(Data::JSON& token, ContextScope const& context, std::size_t recursionDepth) const ;

    /**
     * @brief Handles the evaluation of an eval component.
     * @param token The string to populate with the evaluated value.
     */
    void evalComponentTypeEval(std::string& token) const ;

    //------------------------------------------
    // Evaluation

    [[nodiscard]] double evalAsDoubleImpl() const ;

    [[nodiscard]] Data::JSON evalAsJsonImpl(ContextScope const& context, std::size_t recursionDepth) const ;
};

} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSIONCOMPONENT_HPP
