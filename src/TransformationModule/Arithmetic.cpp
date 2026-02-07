#include "TransformationModule/Arithmetic.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Arithmetic::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&Arithmetic::add, addName, addDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::mod, modName, modDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::multiply, multiplyName, multiplyDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::pow, powName, powDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::subtract, subtractName, subtractDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::divide, divideName, divideDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::sqrt, sqrtName, sqrtDesc);
    BIND_TRANSFORMATION_STATIC(&Arithmetic::root, rootName, rootDesc);
}

bool Arithmetic::add(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const num = std::stod(args[1]);
        jsonDoc->set_add(rootKey, num);
    } catch (...) {
        return false;
    }
    return true;
}

bool Arithmetic::multiply(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const num = std::stod(args[1]);
        jsonDoc->set_multiply(rootKey, num);
    } catch (...) {
        return false;
    }
    return true;
}

bool Arithmetic::mod(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const modValue = std::stod(args[1]);
        auto const currentValue = jsonDoc->get<double>(rootKey, 0.0);
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
            return false; // Modulo by zero is undefined
        }
        double const result = std::fmod(currentValue, modValue);
        jsonDoc->set<double>(rootKey, result);
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
        auto const currentValue = jsonDoc->get<double>(rootKey, 0.0);
        auto const exponent = std::stod(args[1]);
        double const result = std::pow(currentValue, exponent);
        jsonDoc->set<double>(rootKey, result);
        return true;
    } catch (...) {
        return false;
    }
}

bool Arithmetic::subtract(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const num = - std::stod(args[1]);
        jsonDoc->set_add(rootKey, num);
    } catch (...) {
        return false;
    }
    return true;
}

bool Arithmetic::divide(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const num = 1.0 / std::stod(args[1]);
        jsonDoc->set_multiply(rootKey, num);
    } catch (...) {
        return false;
    }
    return true;
}

bool Arithmetic::sqrt(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 1) {
        return false;
    }
    try {
        auto const currentValue = jsonDoc->get<double>(rootKey, 0.0);
        if (currentValue < 0.0) {
            return false; // Square root of negative number is undefined
        }
        double const result = std::sqrt(currentValue);
        jsonDoc->set<double>(rootKey, result);
        return true;
    } catch (...) {
        return false;
    }
}

bool Arithmetic::root(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 2) {
        return false;
    }
    try {
        double const n = std::stod(args[1]);
        if (std::fabs(n) < std::numeric_limits<double>::epsilon()) {
            return false; // Root of order zero is undefined
        }
        auto const currentValue = jsonDoc->get<double>(rootKey, 0.0);
        if (currentValue < 0.0 && std::fabs(std::fmod(n, 2.0)) < std::numeric_limits<double>::epsilon()) {
            return false; // Even root of negative number is undefined
        }
        double const result = std::pow(currentValue, 1.0 / n);
        jsonDoc->set<double>(rootKey, result);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace Nebulite::TransformationModule
