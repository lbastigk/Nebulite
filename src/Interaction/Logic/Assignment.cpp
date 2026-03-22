#include "Interaction/Logic/Assignment.hpp"

#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Data/Document/JsonScope.hpp"

namespace Nebulite::Interaction::Logic {

void Assignment::setValueOfKey(Data::ScopedKeyView const& keyEvaluated, std::string const& val, Data::JsonScope& target) const {
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
            Global::capture().error.println("Could not determine context from key, skipping assignment");
            break;
        default:
            std::unreachable();
    }
}

void Assignment::setValueOfKey(Data::ScopedKeyView const& keyEvaluated, double const& val, Data::JsonScope& target) const {
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
            Global::capture().error.println("Could not determine context from key, skipping assignment");
            break;
        default:
            std::unreachable();
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
            Global::capture().error.println("Unsupported operation: concat. If you see this message, something is wrong with the deserialization process of a Ruleset!");
            break;
        case Operation::null:
            Global::capture().error.println("Could not determine context from key, skipping assignment");
            break;
        default:
            std::unreachable();
    }
}

void Assignment::apply(ContextScope const& context) const {
    //------------------------------------------
    // Check what the target document to apply the ruleset to is

    Data::JsonScope* targetDocument;
    switch (onType) {
        case Type::Self:
            targetDocument = &context.self;
            break;
        case Type::Other:
            targetDocument = &context.other;
            break;
        case Type::Global:
            targetDocument = &context.global;
            break;
        case Type::null:
            // TODO: determine context from expression!
            // If still null, skip assignment
            Global::capture().error.println("Assignment expression has null type - skipping");
            return; // Skip this expression
        default:
            std::unreachable();
    }

    //------------------------------------------
    // Update

    // If the expression is returnable as double, we can optimize numeric operations
    if (expression->isReturnableAsDouble()) {
        double const resolved = expression->evalAsDouble(context);
        if (targetValuePtr != nullptr) {
            setValueOfKey(resolved, targetValuePtr);
        } else {
            // Target is not associated with a direct double pointer
            // Likely because the target is in document other

            // Try to get a stable double pointer from the target document
            auto const scopedKey = Data::ScopedKey(key->eval(context));
            if (double* target = targetDocument->getStableDoublePointer(scopedKey.view()); target != nullptr) {
                // Lock is needed here, otherwise we have race conditions, and the engine is no longer deterministic!
                auto lock(targetDocument->lock());
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
        std::string const resolved = expression->eval(context);
        auto const k = Data::ScopedKey(key->eval(context));
        setValueOfKey(k.view(), resolved, *targetDocument);
    }
}
} // namespace Nebulite::Interaction::Logic
