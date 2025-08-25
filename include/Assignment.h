/**
 * @file Assignment.h
 * 
 * This file contains the Assignment struct, used to represent variable assignments in the Nebulite scripting language.
 */

#include "ExpressionPool.h"

#pragma once

namespace Nebulite {

/**
 * @class Nebulite::Assignment
 * @brief Struct representing a variable assignment in the Nebulite scripting language.
 * 
 * Contains:
 * 
 * - Type of operation used
 * 
 * - Target document type (Self, Other, Global)
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
    enum class Operation {null, set,add,multiply,concat};

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
     * @brief Represents the full assignment as string
     * 
     * e.g. "0", "$($(self.posX) + 1)"
     * 
     * Storing the full value is necessary for:
     * 
     * - estimating computational cost based on the amount of evaluations `$` as well as variables `{...}`
     * - parsing the expression later on
     * 
     * @todo Is it possible to instead use expression directly? 
     * Since expression stores the full string as well
     */
    std::string value;

    /**
     * @brief The parsed expression in a thread-friendly Pool-Configuration
     */
    Nebulite::ExpressionPool expression;

    //---------------------------------------
    // Disabling copy, allowing move

    // Disable copy constructor and assignment
    Assignment(const Assignment&) = delete;
    Assignment& operator=(const Assignment&) = delete;

    // Enable move constructor and assignment
    Assignment() = default;
    Assignment(Assignment&& other) noexcept
        : operation(other.operation)
        , onType(other.onType)
        , key(std::move(other.key))
        , value(std::move(other.value))
        , expression(std::move(other.expression))
    {
    }

    Assignment& operator=(Assignment&& other) noexcept {
        if (this != &other) {
            operation = other.operation;
            onType = other.onType;
            key = std::move(other.key);
            value = std::move(other.value);
            expression = std::move(other.expression);
        }
        return *this;
    }
};

} // namespace Nebulite
