/**
 * @file Assignment.hpp
 * @brief This file contains the Assignment struct, used to represent
 *        string-defined variable assignments.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP
#define NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Context.hpp"
#include "Interaction/Logic/ExpressionPool.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Interaction::Rules::Construction {
class RulesetCompiler;
} // namespace Nebulite::Interaction::Rules::Construction

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @struct Nebulite::Interaction::Logic::Assignment
 * @brief Representing a variable assignment in the Nebulite scripting language.
 * @details [target] [operation] [value-to-evaluate]
 *          e.g.:
 *          - `self.posX = 0`
 *          - `other.health += $(self.damage * 2)`
 *          - `global.name |= " the Great"`
 */
class Assignment{
public:
    //------------------------------------------
    // Standard constructor/destructor

    Assignment() = default;
    ~Assignment() = default;

    //------------------------------------------
    // Parsing

    /**
     * @brief Parses the assignment from a string.
     * @param str The string containing the assignment
     * @return true on success, false on failure
     */
    bool parse(std::string_view const& str);

    /**
     * @brief Tries to optimize the assignment by assuming the provided context self and global are constant.
     * @param contextScope The context to use for optimization, containing the self and global context.
     * @todo Just like expression, we could auto-optimize to use the first provided context
     */
    void optimize(ContextScope const& contextScope);

    //------------------------------------------
    // Since assignments are unique to a RenderObject

    // disable copying
    Assignment(Assignment const&) = delete;
    Assignment& operator=(Assignment const&) = delete;

    // enable moving
    Assignment(Assignment&&) noexcept = default;
    Assignment& operator=(Assignment&&) noexcept = delete;

    //------------------------------------------

    /**
     * @brief Applies the assignment to the given target document.
     */
    void apply(ContextScope const& context) const ;

    /**
     * @brief Get the unevaluated expression as string
     */
    [[nodiscard]] std::string const& getFullExpression() const {
        return value;
    }

    /**
     * @brief Type of operation used
     */
    enum class Operation : uint8_t {
        null,       // No operation
        set,        // '='
        add,        // '+='
        multiply,   // '*='
        concat      // '|='
    };

private:
    void setValueOfKey(Data::ScopedKeyView const& keyEvaluated, std::string const& val, Data::JsonScope& target) const ;
    void setValueOfKey(Data::ScopedKeyView const& keyEvaluated, double const& val, Data::JsonScope& target) const ;
    void setValueOfKey(double const& val, double* target) const ;

    /**
     * @brief Target document type (Self, Other, Global)
     * @details Depending on Type, the proper JSON document will be used.
     *          Initialized as resource, which means the assignment is evaluated at runtime.
     */
    ContextDeriver::TargetType onType = ContextDeriver::TargetType::resource;

    /**
     * @brief Key of the variable being assigned
     * @details e.g.: "posX"
     */
    std::string keyStr;

    /**
     * @brief Parsed expression representing the key
     */
    std::unique_ptr<Expression> key;

    /**
     * @brief Represents the full assignment as string
     * @details e.g. "0", "$($(self.posX) + 1)", does not include the assignment operator and target
     *          Storing the full value is necessary for:
     *          - estimating computational cost based on the amount of evaluations `$` as well as variables `{...}`
     *          - parsing the expression later on
     */
    std::string value;

    // Activate threadsafe expression pool only if needed
#if EXPRESSION_POOL_SIZE > 1
    /**
     * @brief The parsed expression in a thread-friendly Pool-Configuration
     */
    std::unique_ptr<ExpressionPool> expression;
#else // EXPRESSION_POOL_SIZE > 1
    static_assert(EXPRESSION_POOL_SIZE == 1, "EXPRESSION_POOL_SIZE must be at least 1");
    std::unique_ptr<Expression> expression;
#endif // EXPRESSION_POOL_SIZE > 1

    /**
     * @brief Expression assignment target as double pointer
     * @details Is only unequal to nullptr if:
     *          - onType is Self
     *          - operation is numeric (add, multiply)
     *          - expression is returnable as double
     */
    double* targetValuePtr = nullptr;

    /**
     * @brief Type of operation used.
     * @details Depending on operation, the proper JSON operation helper will be called.
     *          This ensures quick and threadsafe assignment.
     *          Initialized as null, which means the assignment is ignored.
     */
    Operation operation = Operation::null;
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP
