//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <limits>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Module/Transformation/Arithmetic.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {

double currentValueOrNaN(Data::JsonScope const& jsonDoc, Data::ScopedKeyView const& key) {
    return jsonDoc.get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
}

} // namespace

void Arithmetic::bindTransformations() {
    bindTransformation(&Arithmetic::add, addName, addDesc);
    bindTransformation(&Arithmetic::mod, modName, modDesc);
    bindTransformation(&Arithmetic::multiply, multiplyName, multiplyDesc);
    bindTransformation(&Arithmetic::pow, powName, powDesc);
    bindTransformation(&Arithmetic::subtract, subtractName, subtractDesc);
    bindTransformation(&Arithmetic::divide, divideName, divideDesc);
    bindTransformation(&Arithmetic::sqrt, sqrtName, sqrtDesc);
    bindTransformation(&Arithmetic::root, rootName, rootDesc);
}

bool Arithmetic::add(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const num, Data::ScopedKeyView const& key) {
        jsonDoc.set<double>(key, currentValueOrNaN(jsonDoc, key) + num);
        return true;
    });
}

bool Arithmetic::multiply(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const num, Data::ScopedKeyView const& key) {
        jsonDoc.set<double>(key, currentValueOrNaN(jsonDoc, key) * num);
        return true;
    });
}

bool Arithmetic::mod(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const modValue, Data::ScopedKeyView const& key) {
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
            return false; // Modulo by zero is undefined
        }
        double const result = std::fmod(currentValueOrNaN(jsonDoc, key), modValue);
        jsonDoc.set<double>(key, result);
        return true;
    });
}

bool Arithmetic::pow(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const exponent, Data::ScopedKeyView const& key) {
        double const result = std::pow(currentValueOrNaN(jsonDoc, key), exponent);
        jsonDoc.set<double>(key, result);
        return true;
    });
}

bool Arithmetic::subtract(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const num, Data::ScopedKeyView const& key) {
        jsonDoc.set<double>(key, currentValueOrNaN(jsonDoc, key) - num);
        return true;
    });
}

bool Arithmetic::divide(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const divisor, Data::ScopedKeyView const& key) {
        if (std::fabs(divisor) < std::numeric_limits<double>::epsilon()) {
            return false; // Division by zero is undefined
        }
        double const result = currentValueOrNaN(jsonDoc, key) / divisor;
        jsonDoc.set<double>(key, result);
        return true;
    });
}

bool Arithmetic::root(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, [&jsonDoc](double const n, Data::ScopedKeyView const& key) {
        if (std::fabs(n) < std::numeric_limits<double>::epsilon()) {
            return false; // Root of order zero is undefined
        }
        auto const currentValue = currentValueOrNaN(jsonDoc, key);
        if (currentValue < 0.0 && std::fabs(std::fmod(n, 2.0)) < std::numeric_limits<double>::epsilon()) {
            return false; // Even root of negative number is undefined
        }
        double const result = std::pow(currentValue, 1.0 / n);
        jsonDoc.set<double>(key, result);
        return true;
    });
}

bool Arithmetic::sqrt(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 1) {
        return false; // No arguments should be provided for sqrt, as it's an operator with a single operand (the current JSON value)
    }
    auto const currentValue = currentValueOrNaN(jsonDoc, rootKey);
    jsonDoc.set<double>(rootKey, std::sqrt(currentValue));
    return true;
}

} // namespace Nebulite::Module::Transformation
