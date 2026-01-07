/**
 * @file Assignment.hpp
 * 
 * This file contains the Assignment struct, used to represent 
 * variable assignments in the Nebulite scripting language.
 */

#ifndef NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP
#define NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP

//------------------------------------------
// Includes

// Nebulite
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
 * @details [target] [operation] [value]
 */
class Assignment{
public:
    //------------------------------------------
    // Standard constructor/destructor
    Assignment() = default;
    ~Assignment() = default;

    //------------------------------------------
    // Since assignments are unique to a RenderObject

    // disable copying
    Assignment(Assignment const&) = delete;
    Assignment& operator=(Assignment const&) = delete;

    // enable moving
    Assignment(Assignment&&) noexcept = default;
    Assignment& operator=(Assignment&&) noexcept = delete;

    //------------------------------------------
    // Allow ruleset compiler to access private members
    friend class Nebulite::Interaction::Rules::Construction::RulesetCompiler;

    //------------------------------------------

    /**
     * @brief Applies the assignment to the given target document.
     */
    void apply(Core::JsonScope& self, Core::JsonScope& other);

    /**
     * @brief Get the unevaluated expression as string
     */
    [[nodiscard]] std::string const& getFullExpression() const {
        return value;
    }

private:
    void setValueOfKey(Data::ScopedKeyView const& keyEvaluated, std::string const& value, Core::JsonScope& target) const ;
    void setValueOfKey(Data::ScopedKeyView const& keyEvaluated, double const& value, Core::JsonScope& target) const ;
    void setValueOfKey(double const& value, double* target) const ;

    /**
     * @brief Target document type (Self, Other, Global)
     */
    enum class Type : uint8_t {
        null,
        Self,
        Other,
        Global
    };

    /**
     * @brief Target document type (Self, Other, Global)
     * @details Depending on Type, the proper JSON document will be used.
     *          Initialized as null, which means the assignment is evaluated at runtime.
     */
    Type onType = Type::null;

    /**
     * @brief Key of the variable being assigned
     *
     * e.g.: "posX"
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
#else
    // Throw error
    #error "EXPRESSION_POOL_SIZE must be greater than 1 to use Assignment expression pools."
#endif

    /**
     * @brief Expression assignment target as double pointer
     *
     * Is only unequal to nullptr if:
     * - onType is Self
     * - operation is numeric (add, multiply)
     * - expression is returnable as double
     */
    double* targetValuePtr = nullptr;

    /**
     * @brief Type of operation used
     */
    enum class Operation : uint8_t {
        null,
        set,
        add,
        multiply,
        concat
    };

    /**
     * @brief Type of operation used.
     *
     * Depending on operation, the proper JSON operation helper will be called.
     * This ensures quick and threadsafe assignment.
     *
     * Initialized as null, which means the assignment is ignored.
     */
    Operation operation = Operation::null;
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP
