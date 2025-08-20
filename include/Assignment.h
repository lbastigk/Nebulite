#include "ExpressionPool.h"

#pragma once

namespace Nebulite {
struct Assignment{
    enum class Operation {null, set,add,multiply,concat};
    Operation operation = Operation::null;                      // set, add, multiply, concat
    enum class Type {null, Self, Other, Global};
    Type onType = Type::null;                              // Self, Other, Global, determines which doc is used
    std::string key;                          // e.g. "posX"
    std::string value;                        // e.g. "0", "$($(self.posX) + 1)"
    Nebulite::ExpressionPool expression;    // The parsed expression

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
