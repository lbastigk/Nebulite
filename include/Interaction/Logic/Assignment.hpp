/**
 * @file Assignment.hpp
 * @brief This file contains the Assignment struct, used to represent
 *        string-defined variable assignments.
 */

#ifndef INTERACTION_LOGIC_ASSIGNMENT_HPP
#define INTERACTION_LOGIC_ASSIGNMENT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Context.hpp"
#include "Interaction/Logic/Expression.hpp"

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
 * @class Nebulite::Interaction::Logic::Assignment
 * @brief Representing a variable assignment in the Nebulite scripting language.
 * @details [target] [operation] [value-to-evaluate]
 *          e.g.:
 *          - self:posX = 0
 *          - other:health += $(self:damage * 2)
 *          - global:name |= " the Great"
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
    [[nodiscard]] std::string const& getFullExpression() const ;

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
    void setValueOfKey(Data::ScopedKeyView const& keyEvaluated, double val, Data::JsonScope& target) const ;
    void setValueOfKey(Data::ScopedKeyView const& keyEvaluated, int64_t val, Data::JsonScope& target) const ;
    void setValueOfKey(double val, double* target) const ;

    /**
     * @brief Target document type (Self, Other, Global)
     * @details Depending on Type, the proper JSON document will be used.
     *          Initialized as resource, which means the assignment is evaluated at runtime.
     */
    ContextDeriver::TargetType onType = ContextDeriver::TargetType::resource;

    /**
     * @brief Parsed expression representing the key
     */
    std::unique_ptr<Expression> key;

    /**
     * @brief Parsed expression representing the value to assign
     */
    std::unique_ptr<Expression> expression;

    /**
     * @brief Expression assignment target as double pointer
     * @details Is only unequal to nullptr if:
     *          - onType is Self
     *          - operation is numeric (set, add, multiply)
     *          - expression is returnable as double
     */
    double* targetValuePtr = nullptr;

    /**
     * @brief Type of assignment operation used.
     * @details Depending on operation, the proper JSON operation helper will be called.
     *          This ensures quick and threadsafe assignment.
     *          Initialized as null, which means the assignment is ignored.
     */
    Operation operation = Operation::null;
};
} // namespace Nebulite::Interaction::Logic
#endif // INTERACTION_LOGIC_ASSIGNMENT_HPP
