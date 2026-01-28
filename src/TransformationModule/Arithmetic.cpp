#include "TransformationModule/Arithmetic.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Arithmetic::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Arithmetic::add, addName, addDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::mod, modName, modDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::multiply, multiplyName, multiplyDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::pow, powName, powDesc);
}

bool Arithmetic::add(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    for (auto const& numStr : args.subspan(1)) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_add(valueKey, num);
        } catch (...) {
            return false;
        }
    }
    return true;
}

bool Arithmetic::multiply(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    for (auto const numbers = args.subspan(1); auto const& numStr : numbers) {
        try {
            double num = std::stod(numStr);
            jsonDoc->set_multiply(valueKey, num);
        } catch (...) {
            return false;
        }
    }
    return true;
}

bool Arithmetic::mod(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    try {
        double const modValue = std::stod(args[1]);
        auto const currentValue = jsonDoc->get<double>(valueKey, 0.0);
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
            return false; // Modulo by zero is undefined
        }
        double const result = std::fmod(currentValue, modValue);
        jsonDoc->set<double>(valueKey, result);
        return true;
    } catch (...) {
        return false;
    }
}

bool Arithmetic::pow(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const exponent = std::stod(args[1]);
        auto const currentValue = jsonDoc->get<double>(valueKey, 0.0);
        double const result = std::pow(currentValue, exponent);
        jsonDoc->set<double>(valueKey, result);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace Nebulite::TransformationModule
