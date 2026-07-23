#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSIONCOMPONENT_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSIONCOMPONENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <utility>

// External
#include <tinyexpr.h>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Formatter.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @struct Nebulite::Interaction::Logic::ExpressionComponent
 * @brief Represents a single component in an expression, such as a variable, evaluation, or text.
 * @details Holds information about a specific part of the expression,
 *          including its type, source, and any associated metadata.
 */
class ExpressionComponent {
public:
    /**
     * @enum Nebulite::Interaction::Logic::ExpressionComponent::Type
     * @brief Each component can be of type variable, eval or text that differ in how they are evaluated.
     */
    enum class Type : std::uint8_t {
        variable, // outside $<cast>(...), Starts with self, other, global or a dot for link, represents a variable reference, outside an evaluatable context
        eval, // inside a $<cast>(...), represents an evaluatable expression
        text // outside a $<cast>(...), not a variable reference, Represents a plain text string
    } type = Type::text;

    ContextDeriver::TargetType contextType = ContextDeriver::TargetType::none; // Default to none

    Formatter formatter; // Formatting options for this component, if applicable

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

    //------------------------------------------
    // Component handling methods

    /**
     * @brief Handles the evaluation of a variable component as a string.
     * @details Takes care of proper conversion to string, with abbreviated representations for non-value types (arrays, objects and null).
     * @param token The string to populate with the evaluated value.
     * @param context The context to evaluate against.
     * @param recursionDepth The current recursion depth for nested evaluations.
     * @return True if the evaluation was successful, false otherwise.
     */
    bool handleComponentTypeVariable(std::string& token, ContextScope const& context, std::size_t recursionDepth) const ;

    /**
     * @brief Handles the evaluation of a variable component as a JSON value.
     * @details Populates the provided JSON object with the evaluated value, preserving its type.
     * @param token The JSON object to populate with the evaluated value.
     * @param context The context to evaluate against.
     * @param recursionDepth The current recursion depth for nested evaluations.
     * @return True if the evaluation was successful, false otherwise.
     */
    bool handleComponentTypeVariable(Data::JSON& token, ContextScope const& context, std::size_t recursionDepth) const ;

    /**
     * @brief Handles the evaluation of an eval component.
     * @param token The string to populate with the evaluated value.
     */
    void handleComponentTypeEval(std::string& token) const ;

private:
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
};

} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSIONCOMPONENT_HPP
