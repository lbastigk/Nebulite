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
#include "Nebulite.hpp"
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
struct alignas(DUAL_CACHE_LINE_ALIGNMENT) Assignment{
    //------------------------------------------
    // Standard constructor/destructor
    Assignment() = default;
    ~Assignment() = default;

    //------------------------------------------
    // Since assignments are unique to a RenderObject
    // Disabling copy, allowing move for certain operations

    /**
     * @todo Using shared_ptr for Assignments so that even move operations are not necessary
     */

    // Disable copy constructor and assignment
    Assignment(Assignment const&) = delete;
    Assignment& operator=(Assignment const&) = delete;

    // Enable move constructor and assignment
    Assignment(Assignment&& other) noexcept = default;
    Assignment& operator=(Assignment&& other) noexcept = default;

    //------------------------------------------

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
     * @brief A unique id of the key in the target document
     * 
     * Used for quick access to a target value pointer in the target document.
     */
    uint64_t targetKeyUniqueId = 0;

    /**
     * @brief if the unique id was already initialized
     */
    bool targetKeyUniqueIdInitialized = false;

    
    
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
