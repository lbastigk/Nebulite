#include "Interaction/Logic/Assignment.hpp"

#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Interaction::Logic {

void Assignment::setValueOfKey(Data::ScopedKeyView const& keyEvaluated, std::string const& val, Data::JsonScopeBase& target) const {
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation) {
    case Operation::set:
        target.set<std::string>(keyEvaluated, val);
        break;
    case Operation::add:
        target.set_add(keyEvaluated, std::stod(val));
        break;
    case Operation::multiply:
        target.set_multiply(keyEvaluated, std::stod(val));
        break;
    case Operation::concat:
        target.set_concat(keyEvaluated, val);
        break;
    case Operation::null:
        Error::println("Could not determine context from key, skipping assignment");
        break;
    default:
        Error::println("Unknown operation type! Enum value:", static_cast<int>(operation));
        break;
    }
}

void Assignment::setValueOfKey(Data::ScopedKeyView const& keyEvaluated, double const& val, Data::JsonScopeBase& target) const {
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation) {
    case Operation::set:
        target.set<double>(keyEvaluated, val);
        break;
    case Operation::add:
        target.set_add(keyEvaluated, val);
        break;
    case Operation::multiply:
        target.set_multiply(keyEvaluated, val);
        break;
    case Operation::concat:
        target.set_concat(keyEvaluated, std::to_string(val));
        break;
    case Operation::null:
        Error::println("Could not determine context from key, skipping assignment");
        break;
    default:
        Error::println("Unknown operation type! Enum value:", static_cast<int>(operation));
        break;
    }
}

void Assignment::setValueOfKey(double const& val, double* target) const {
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation) {
    case Operation::set:
        *target = val;
        break;
    case Operation::add:
        *target += val;
        break;
    case Operation::multiply:
        *target *= val;
        break;
    case Operation::concat:
        Error::println("Unsupported operation: concat. If you see this message, something is wrong with the deserialization process of an Invoke!");
        break;
    case Operation::null:
        Error::println("Could not determine context from key, skipping assignment");
        break;
    default:
        Error::println("Unknown operation type! Enum value:", static_cast<int>(operation));
        break;
    }
}

void Assignment::apply(Data::JsonScopeBase& self, Data::JsonScopeBase& other) {
    //------------------------------------------
    // Check what the target document to apply the ruleset to is

    Data::JsonScopeBase* targetDocument;
    switch (onType) {
    case Type::Self:
        targetDocument = &self;
        break;
    case Type::Other:
        targetDocument = &other;
        break;
    case Type::Global:
        targetDocument = &Global::instance().domainScope;
        break;
    case Type::null:
        // TODO: determine context from expression!
        // If still null, skip assignment
        Error::println("Assignment expression has null type - skipping");
        return; // Skip this expression
    default:
        Error::println("Unknown assignment type: ", static_cast<int>(onType), " - skipping");
        return; // Exit if unknown type
    }

    //------------------------------------------
    // Update

    // If the expression is returnable as double, we can optimize numeric operations
    if (expression->isReturnableAsDouble()) {
        double const resolved = expression->evalAsDouble(other);
        if (targetValuePtr != nullptr) {
            setValueOfKey(resolved, targetValuePtr);
        } else {
            // Target is not associated with a direct double pointer
            // Likely because the target is in document other

            // Try to get a stable double pointer from the target document
            auto const scopedKey = Data::ScopedKey(key->eval(other));
            if (double* target = targetDocument->getStableDoublePointer(scopedKey.view()); target != nullptr) {
                // Lock is needed here, otherwise we have race conditions, and the engine is no longer deterministic!
                std::scoped_lock lock(targetDocument->lock());
                setValueOfKey(resolved, target);
            } else {
                // Still not possible, fallback to using JSON's internal methods
                // This is slower, but should work in all cases
                // No lock needed here, as we use JSON's threadsafe methods
                setValueOfKey(scopedKey.view(), resolved, *targetDocument);
            }
        }
    }
    // If not, we resolve as string and update that way
    else {
        std::string const resolved = expression->eval(other);
        auto const k = Data::ScopedKey(key->eval(other));
        setValueOfKey(k.view(), resolved, *targetDocument);
    }
}
} // namespace Nebulite::Interaction::Logic
