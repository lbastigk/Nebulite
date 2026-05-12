//------------------------------------------
// Includes

// Standard library
#include <string_view>

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

namespace {

struct OperationInfo {
    Assignment::Operation op;
    std::string_view symbol;
    constexpr OperationInfo(Assignment::Operation const& o, std::string_view const& s) noexcept : op(o), symbol(s) {}
};

std::array constexpr supportedOperations = {
    OperationInfo{Assignment::Operation::add, "+="},
    OperationInfo{Assignment::Operation::multiply, "*="},
    OperationInfo{Assignment::Operation::concat, "|="},
    OperationInfo{Assignment::Operation::set, "="} // Must come at last place, otherwise it is registered before any other operator
};

} // namespace

bool Assignment::parse(std::string_view const& str) {
    // 1.) Derive context
    onType = ContextDeriver::getTypeFromString(str);
    if (onType == ContextDeriver::TargetType::resource) {
        return false;
    }

    // 2.) Strip context, create views for key and value
    std::string_view valueView = str;
    ContextDeriver::stripContextFromView(valueView);
    std::string_view keyView = str;
    ContextDeriver::stripContextFromView(keyView);

    // 3.) Find the operator position, get views for key and value
    operation = Operation::null;
    for (auto const& [op, symbol] : supportedOperations) {
        if (size_t const pos = keyView.find(symbol); pos != std::string::npos) {
            operation = op;
            valueView.remove_prefix(pos + symbol.size());
            keyView.remove_suffix(keyView.size() - pos);
            break;
        }
    }
    if (operation == Operation::null) { // No supported operation found
        return false;
    }

    // 4.) Generate expressions for key and value
    Utility::StringHandler::strip(keyView);
    Utility::StringHandler::strip(valueView);
    key = std::make_unique<Expression>(keyView);
    expression = std::make_unique<Expression>(valueView);

    return true;
}

void Assignment::optimize(ContextScope const& contextScope){
    // Supported operations for optimizations
     std::array constexpr numeric_operations = {
         Operation::set,
         Operation::add,
         Operation::multiply
     };

    // Keys with transformations cannot be optimized to use a stable double pointer
    if (key->getFullExpression().contains(Data::JSON::SpecialCharacter::transformationPipe)) {
        return;
    }

    // Optimize supported targets from a given context
    // - target self is the owner and stays the same
    // - target global is assumed to stay the same
    // We cannot optimize for target other, as that changes based on the interaction.
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

std::string const& Assignment::getFullExpression() const {
    return expression->getFullExpression();
}

} // namespace Nebulite::Interaction::Logic
