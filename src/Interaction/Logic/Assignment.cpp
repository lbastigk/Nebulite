#include "Interaction/Logic/Assignment.hpp"

#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::Interaction::Logic {

void Assignment::setValueOfKey(Data::ScopedKey const& keyEvaluated, std::string const& val, Core::JsonScope& target) const {
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation) {
    case Logic::Assignment::Operation::set:
        target.set<std::string>(keyEvaluated, val);
        break;
    case Logic::Assignment::Operation::add:
        target.set_add(keyEvaluated, std::stod(val));
        break;
    case Logic::Assignment::Operation::multiply:
        target.set_multiply(keyEvaluated, std::stod(val));
        break;
    case Logic::Assignment::Operation::concat:
        target.set_concat(keyEvaluated, val);
        break;
    case Logic::Assignment::Operation::null:
        Nebulite::cerr() << "Could not determine context from key, skipping assignment" << Nebulite::endl;
        break;
    default:
        Nebulite::cerr() << "Unknown operation type! Enum value:" << static_cast<int>(operation) << Nebulite::endl;
        break;
    }
}

void Assignment::setValueOfKey(Data::ScopedKey const& keyEvaluated, double const& val, Core::JsonScope& target) const {
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation) {
    case Logic::Assignment::Operation::set:
        target.set<double>(keyEvaluated, val);
        break;
    case Logic::Assignment::Operation::add:
        target.set_add(keyEvaluated, val);
        break;
    case Logic::Assignment::Operation::multiply:
        target.set_multiply(keyEvaluated, val);
        break;
    case Logic::Assignment::Operation::concat:
        target.set_concat(keyEvaluated, std::to_string(val));
        break;
    case Logic::Assignment::Operation::null:
        Nebulite::cerr() << "Could not determine context from key, skipping assignment" << Nebulite::endl;
        break;
    default:
        Nebulite::cerr() << "Unknown operation type! Enum value:" << static_cast<int>(operation) << Nebulite::endl;
        break;
    }
}

void Assignment::setValueOfKey(double const& val, double* target) const {
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation) {
    case Logic::Assignment::Operation::set:
        *target = val;
        break;
    case Logic::Assignment::Operation::add:
        *target += val;
        break;
    case Logic::Assignment::Operation::multiply:
        *target *= val;
        break;
    case Logic::Assignment::Operation::concat:
        Nebulite::cerr() << "Unsupported operation: concat. If you see this message, something is wrong with the deserialization process of an Invoke!" << Nebulite::endl;
        break;
    case Logic::Assignment::Operation::null:
        Nebulite::cerr() << "Could not determine context from key, skipping assignment" << Nebulite::endl;
        break;
    default:
        Nebulite::cerr() << "Unknown operation type! Enum value:" << static_cast<int>(operation) << Nebulite::endl;
        break;
    }
}

void Assignment::apply(Core::JsonScope& self, Core::JsonScope& other) {
    //------------------------------------------
    // Check what the target document to apply the ruleset to is

    Core::JsonScope* targetDocument;
    switch (onType) {
    case Logic::Assignment::Type::Self:
        targetDocument = &self;
        break;
    case Logic::Assignment::Type::Other:
        targetDocument = &other;
        break;
    case Logic::Assignment::Type::Global:
        targetDocument = &Nebulite::global().getDoc();
        break;
    case Logic::Assignment::Type::null:
        // TODO: determine context from expression!
        // If still null, skip assignment
        Nebulite::cerr() << "Assignment expression has null type - skipping" << Nebulite::endl;
        return; // Skip this expression
    default:
        Nebulite::cerr() << "Unknown assignment type: " << static_cast<int>(onType) << Nebulite::endl;
        return; // Exit if unknown type
    }

    //------------------------------------------
    // Update

    // If the expression is returnable as double, we can optimize numeric operations
    if (expression.isReturnableAsDouble()) {
        double const resolved = expression.evalAsDouble(other);
        if (targetValuePtr != nullptr) {
            setValueOfKey(resolved, targetValuePtr);
        } else {
            // Target is not associated with a direct double pointer
            // Likely because the target is in document other

            // Try to get a stable double pointer from the target document
            if (double* target = targetDocument->getStableDoublePointer(Data::ScopedKey(key.eval(other))); target != nullptr) {
                // Lock is needed here, otherwise we have race conditions, and the engine is no longer deterministic!
                std::scoped_lock lock(targetDocument->lock());
                setValueOfKey(resolved, target);
            } else {
                // Still not possible, fallback to using JSON's internal methods
                // This is slower, but should work in all cases
                // No lock needed here, as we use JSON's threadsafe methods
                setValueOfKey(Data::ScopedKey(key.eval(other)), resolved, *targetDocument);
            }
        }
    }
    // If not, we resolve as string and update that way
    else {
        std::string const resolved = expression.eval(other);
        setValueOfKey(Data::ScopedKey(key.eval(other)), resolved, *targetDocument);
    }
}
} // namespace Nebulite::Interaction::Logic
