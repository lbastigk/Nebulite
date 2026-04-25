#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/Arithmetic.hpp"

namespace Nebulite::Module::Transformation {

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

bool Arithmetic::forall(std::span<std::string const> const& args, std::function<bool(std::string const&, Data::ScopedKeyView const& key)> const& func){
    if (args.size() < 2) {
        return false;
    }
    try {
        if (args.size() == 2) {
            return func(args[1], rootKey);
        }
        return std::ranges::all_of(args | std::views::drop(1) | std::views::enumerate,
            [&](auto const& item) {
                auto const& [index, arg] = item;
                auto key = rootKey + "[" + std::to_string(index) + "]";
                return func(arg, key.view());
            }
        );
    } catch (...) {
        return false;
    }
}

bool Arithmetic::add(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const num = std::stod(arg);
        jsonDoc->set_add(key, num);
        return true;
    });
}

bool Arithmetic::multiply(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const num = std::stod(arg);
        jsonDoc->set_multiply(key, num);
        return true;
    });
}

bool Arithmetic::mod(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const modValue = std::stod(arg);
        auto const currentValue = jsonDoc->get<double>(key).value_or(0.0);
        if (std::fabs(modValue) < std::numeric_limits<double>::epsilon()) {
            return false; // Modulo by zero is undefined
        }
        double const result = std::fmod(currentValue, modValue);
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::pow(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const exponent = std::stod(arg);
        auto const currentValue = jsonDoc->get<double>(key).value_or(0.0);
        double const result = std::pow(currentValue, exponent);
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::subtract(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const num = - std::stod(arg);
        jsonDoc->set_add(key, num);
        return true;
    });
}

bool Arithmetic::divide(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const divisor = std::stod(arg);
        if (std::fabs(divisor) < std::numeric_limits<double>::epsilon()) {
            return false; // Division by zero is undefined
        }
        double const result = jsonDoc->get<double>(key).value_or(0.0) / divisor;
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::root(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    return forall(args, [jsonDoc](std::string const& arg, Data::ScopedKeyView const& key) {
        double const n = std::stod(arg);
        if (std::fabs(n) < std::numeric_limits<double>::epsilon()) {
            return false; // Root of order zero is undefined
        }
        auto const currentValue = jsonDoc->get<double>(key).value_or(0.0);
        if (currentValue < 0.0 && std::fabs(std::fmod(n, 2.0)) < std::numeric_limits<double>::epsilon()) {
            return false; // Even root of negative number is undefined
        }
        double const result = std::pow(currentValue, 1.0 / n);
        jsonDoc->set<double>(key, result);
        return true;
    });
}

bool Arithmetic::sqrt(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 1) {
        return false; // No arguments should be provided for sqrt, as it's an operator with a single operand (the current JSON value)
    }
    try {
        auto const currentValue = jsonDoc->get<double>(rootKey).value_or(0.0);
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

} // namespace Nebulite::Module::Transformation
