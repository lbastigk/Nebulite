//------------------------------------------
// Includes

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

bool Assignment::parse(std::string_view const& str) {
    // Derive context and strip it from the key
    onType = ContextDeriver::getTypeFromString(str);
    if (onType == ContextDeriver::TargetType::resource) {
        return false;
    }
    keyStr = ContextDeriver::stripContext(str);

    std::string_view valueView = keyStr;
    std::string_view keyView = keyStr;
    
    // Find the operator position in the full expression, set operation, key and value
    if (size_t pos; (pos = keyStr.find("+=")) != std::string::npos) {
        operation = Operation::add;
        valueView.remove_prefix(pos + 2);
        keyView.remove_suffix(keyView.size() - pos);
    } else if ((pos = keyStr.find("*=")) != std::string::npos) {
        operation = Operation::multiply;
        valueView.remove_prefix(pos + 2);
        keyView.remove_suffix(keyView.size() - pos);
    } else if ((pos = keyStr.find("|=")) != std::string::npos) {
        operation = Operation::concat;
        valueView.remove_prefix(pos + 2);
        keyView.remove_suffix(keyView.size() - pos);
    } else if ((pos = keyStr.find('=')) != std::string::npos) {
        operation = Operation::set;
        valueView.remove_prefix(pos + 1);
        keyView.remove_suffix(keyView.size() - pos);
    } else {
        return false;
    }
    Utility::StringHandler::strip(keyView);
    Utility::StringHandler::strip(valueView);

    keyStr = keyView;
    key = std::make_unique<Expression>(keyView);
    value = valueView;

    // Set expression
    expression = std::make_unique<Expression>(value);
    return true;
}

void Assignment::optimize(ContextScope const& contextScope){
     std::array constexpr numeric_operations = {
         Operation::set,
         Operation::add,
         Operation::multiply
     };

    // Optimize
    if (key->getFullExpression().find('|') != std::string::npos) {
        // Keys with transformations cannot be optimized to use a stable double pointer
        return;
    }

    if (onType == ContextDeriver::TargetType::self) {
        if (std::ranges::find(numeric_operations, operation) != std::ranges::end(numeric_operations)) {
            // Numeric operation on self, try to get a direct pointer
            targetValuePtr = contextScope.self.getStableDoublePointer(Data::ScopedKey(key->eval(contextScope)));
        }
    }
    if (onType == ContextDeriver::TargetType::global) { // We assume the global context target stays the same
        if (std::ranges::find(numeric_operations, operation) != std::ranges::end(numeric_operations)) {
            // Numeric operation on global, try to get a direct pointer
            targetValuePtr = Global::instance().domainScope.getStableDoublePointer(Data::ScopedKey(key->eval(contextScope)));
        }
    }
}

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
        case Operation::null:
            Global::capture().error.println("Could not determine context from key, skipping assignment");
            break;
        case Operation::concat: // Cannot be reached, is filtered out using isNumericOperation
        default:
            std::unreachable();
    }
}

namespace {

bool isNumericOperation(Assignment::Operation const& op) {
    return op == Assignment::Operation::set || op == Assignment::Operation::add || op == Assignment::Operation::multiply;
}

} // namespace

void Assignment::apply(ContextScope const& context) const {
    //------------------------------------------
    // Check what the target document to apply the ruleset to is
    auto const potentialTarget = context.getTargetFromType(onType);
    if (!potentialTarget.has_value()) {
        Global::capture().error.println("Assignment expression has an unsupported type - skipping");
        return; // Skip this expression
    }
    auto& targetDocument = potentialTarget.value().get();

    //------------------------------------------
    // Update

    // If the expression is returnable as double, we can optimize numeric operations
    if (expression->isReturnableAsDouble() && isNumericOperation(operation)) {
        double const resolved = expression->evalAsDouble(context);
        if (targetValuePtr != nullptr) {
            setValueOfKey(resolved, targetValuePtr);
        } else {
            // Target is not associated with a direct double pointer
            // Likely because the target is in document other

            // Try to get a stable double pointer from the target document
            auto const scopedKey = Data::ScopedKey(key->eval(context));
            if (double* target = targetDocument.getStableDoublePointer(scopedKey.view()); target != nullptr) {
                // Lock is needed here, otherwise we have race conditions, and the engine is no longer deterministic!
                auto lock(targetDocument.lock());
                setValueOfKey(resolved, target);
            } else {
                // Still not possible, fallback to using JSON's internal methods
                // This is slower, but should work in all cases
                // No lock needed here, as we use JSON's threadsafe methods
                setValueOfKey(scopedKey.view(), resolved, targetDocument);
            }
        }
    }
    // Check if returning as a JSON is preferred
    else if (!expression->isReturnableAsString() && this->operation == Operation::set) {
        auto const resolved = expression->evalAsJson(context);
        auto const k = Data::ScopedKey(key->eval(context));
        targetDocument.setSubDoc(k.view(), std::move(resolved));
    }
    // If not, we resolve as string and update that way
    else {
        std::string const resolved = expression->eval(context);
        auto const k = Data::ScopedKey(key->eval(context));
        setValueOfKey(k.view(), resolved, targetDocument);
    }
}
} // namespace Nebulite::Interaction::Logic
