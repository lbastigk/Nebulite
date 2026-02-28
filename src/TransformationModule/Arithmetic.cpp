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

bool Arithmetic::forall(std::span<std::string const> const& args, std::function<bool(std::string const&, Data::ScopedKeyView const& key)> const& func){
    if (args.size() < 2) {
        return false;
    }
    try {
        if (args.size() == 2) {
            return func(args[1], rootKey);
        }
        for (auto [index, arg] : args | std::views::drop(1) | std::views::enumerate) {
            if (auto key = rootKey + "[" + std::to_string(index) + "]";
                !func(arg, key.view()))
            {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool Arithmetic::add(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const num = std::stod(arg);
        jsonDoc->set_add(key, num);
        return true;
    });
}

bool Arithmetic::multiply(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const num = std::stod(arg);
        jsonDoc->set_multiply(key, num);
        return true;
    });
}

bool Arithmetic::mod(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const modValue = std::stod(arg);
        auto const currentValue = jsonDoc->get<double>(key, 0.0);
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
            return false; // Modulo by zero is undefined
        }
        double const result = std::fmod(currentValue, modValue);
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::pow(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const exponent = std::stod(arg);
        auto const currentValue = jsonDoc->get<double>(key, 0.0);
        double const result = std::pow(currentValue, exponent);
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::subtract(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const num = - std::stod(arg);
        jsonDoc->set_add(key, num);
        return true;
    });
}

bool Arithmetic::divide(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const divisor = std::stod(arg);
        if (std::fabs(divisor) < std::numeric_limits<double>::epsilon()) {
            return false; // Division by zero is undefined
        }
        double const result = jsonDoc->get<double>(key, 0.0) / divisor;
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::root(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const n = std::stod(arg);
        if (std::fabs(n) < std::numeric_limits<double>::epsilon()) {
            return false; // Root of order zero is undefined
        }
        auto const currentValue = jsonDoc->get<double>(key, 0.0);
        if (currentValue < 0.0 && std::fabs(std::fmod(n, 2.0)) < std::numeric_limits<double>::epsilon()) {
            return false; // Even root of negative number is undefined
        }
        double const result = std::pow(currentValue, 1.0 / n);
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::sqrt(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 1) {
        return false; // No arguments should be provided for sqrt, as it's an operator with a single operand (the current JSON value)
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

} // namespace Nebulite::TransformationModule
