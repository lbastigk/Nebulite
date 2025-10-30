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
namespace Nebulite::Interaction::Logic {
/**
 * @struct Nebulite::Interaction::Logic::Assignment
 * @brief Representing a variable assignment in the Nebulite scripting language.
 * 
 * Contains:
 * 
 * - Type of operation used
 * 
 * - Target document type (`self`, `other`, `global`)
 * 
 * - Key of the variable being assigned
 * 
 * - Value of the variable being assigned
 * 
 * - The value as parsed expression
 * 
 */
struct Assignment{
    /**
     * @brief Type of operation used
     */
    enum class Operation {null, set, add, multiply, concat};

    /**
     * @brief Type of operation used.
     * 
     * Depending on operation, the proper JSON operation helper will be called.
     * This ensures quick and threadsafe assignment.
     * 
     * Initialized as null, which means the assignment is ignored.
     */
    Operation operation = Operation::null;

    /**
     * @brief Target document type (Self, Other, Global)
     */
    enum class Type {null, Self, Other, Global};

    /**
     * @brief Target document type (Self, Other, Global)
     * 
     * Depending on Type, the proper JSON document will be used.
     * 
     * Initialized as null, which means the assignment is ignored.
     */
    Type onType = Type::null;

    /**
     * @brief Key of the variable being assigned
     * 
     * e.g.: "posX"
     */
    std::string key;

    /**
     * @brief Target value pointer
     * 
     * Is only unequal to nullptr if:
     * - onType is Self
     * - operation is numeric (add, multiply)
     * - expression is returnable as double
     */
    double* targetValuePtr = nullptr;

    /**
     * @brief A unique id of the key in the target document
     * 
     * Used for quick access to a target value pointer in the target document.
     */
    uint64_t targetKeyUniqueId = 0;

    /**
     * @brief if the unique id was already initialized
     */
    bool targetKeyUniqueIdInitialized = false;

    // Activate threadsafe expression pool only if needed
    #if INVOKE_EXPR_POOL_SIZE > 1
        /**
         * @brief The parsed expression in a thread-friendly Pool-Configuration
         */
        Nebulite::Interaction::Logic::ExpressionPool expression;
    #else
        /**
         * @brief The parsed expression
         */
        Nebulite::Interaction::Logic::Expression expression;
    #endif

    //------------------------------------------
    // Disabling copy, allowing move

    // Disable copy constructor and assignment
    Assignment(const Assignment&) = delete;
    Assignment& operator=(const Assignment&) = delete;

    // Enable move constructor and assignment
    Assignment() = default;
    Assignment(Assignment&& other) noexcept : 
        operation(other.operation),
        onType(other.onType),
        key(std::move(other.key)),
        targetValuePtr(other.targetValuePtr),
        targetKeyUniqueId(other.targetKeyUniqueId),
        targetKeyUniqueIdInitialized(other.targetKeyUniqueIdInitialized),
        expression(std::move(other.expression)),
        value(std::move(other.value))
    {
    }

    Assignment& operator=(Assignment&& other) noexcept {
        if (this != &other){
            operation = other.operation;
            onType = other.onType;
            key = std::move(other.key);
            value = std::move(other.value);
            expression = std::move(other.expression);
            targetValuePtr = other.targetValuePtr;
            targetKeyUniqueId = other.targetKeyUniqueId;
            targetKeyUniqueIdInitialized = other.targetKeyUniqueIdInitialized;
        }
        return *this;
    }
    
    /**
     * @brief Represents the full assignment as string
     * 
     * e.g. "0", "$($(self.posX) + 1)", does not include the assignment operator and target
     * 
     * Storing the full value is necessary for:
     * 
     * - estimating computational cost based on the amount of evaluations `$` as well as variables `{...}`
     * - parsing the expression later on
     */
    std::string value;
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_ASSIGNMENT_HPP
