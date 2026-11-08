//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <limits>
#include <optional>

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
    bindTransformation(&Arithmetic::abs, absName, absDesc);
    bindTransformation(&Arithmetic::sqrt, sqrtName, sqrtDesc);
    bindTransformation(&Arithmetic::root, rootName, rootDesc);
}

bool Arithmetic::add(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const num) -> std::optional<double> {
        return input + num;
    });
}

bool Arithmetic::multiply(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const num) -> std::optional<double> {
        return input * num;
    });
}

bool Arithmetic::mod(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const modValue) -> std::optional<double> {
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
            return std::nullopt;
        }
        return std::fmod(input, modValue);
    });
}

bool Arithmetic::pow(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const exponent) -> std::optional<double> {
        return std::isnan(input) ? input : std::pow(input, exponent);
    });
}

bool Arithmetic::subtract(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const num) -> std::optional<double> {
        return input - num;
    });
}

bool Arithmetic::divide(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const divisor) -> std::optional<double> {
        if (std::fabs(divisor) < std::numeric_limits<double>::epsilon()) {
            return std::nullopt;
        }
        return input / divisor;
    });
}

bool Arithmetic::root(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return forall(args, jsonDoc, [](double const input, double const n) -> std::optional<double> {
        if (std::fabs(n) < std::numeric_limits<double>::epsilon()) {
            return std::nullopt;
        }
        if (input < 0.0) {
            if (std::trunc(n) != n || std::fabs(std::fmod(n, 2.0)) != 1.0) {
                return std::nullopt;
            }
            return -std::pow(-input, 1.0 / n);
        }
        return std::pow(input, 1.0 / n);
    });
}

bool Arithmetic::abs(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    if (args.size() != 1) {
        return false; // No arguments should be provided for abs, as it's an operator with a single operand (the current JSON value)
    }
    auto const currentValue = currentValueOrNaN(jsonDoc, rootKey);
    jsonDoc.set<double>(rootKey, std::fabs(currentValue));
    return true;
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
