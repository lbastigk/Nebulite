//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint> // NOLINT
#include <functional>
#include <limits>
#include <map>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Module/Transformation/Statistics.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {

enum class MedianKind : std::uint8_t { conventional, lower, upper };

bool calculateMedian(Data::JsonScope& jsonDoc, MedianKind const kind) {
    if (jsonDoc.memberType(Base::TransformationModule::rootKey) != Data::KeyType::array) {
        return false;
    }

    std::size_t const size = jsonDoc.memberSize(Base::TransformationModule::rootKey);
    if (size == 0) {
        return false;
    }

    std::vector<double> values;
    values.reserve(size);
    for (auto const key : Base::TransformationModule::rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        values.push_back(value);
    }

    std::ranges::sort(values.begin(), values.end());
    double medianValue = 0.0;
    if (size % 2 == 0 && kind == MedianKind::conventional) {
        medianValue = (values[size / 2 - 1] + values[size / 2]) / 2.0;
    } else if (size % 2 == 0 && kind == MedianKind::lower) {
        medianValue = values[size / 2 - 1];
    } else {
        medianValue = values[size / 2];
    }
    jsonDoc.set(Base::TransformationModule::rootKey, medianValue);
    return true;
}

/**
 * @brief Helper function to accumulate values in an array using a provided (double, double) -> double function.
 * @param jsonDoc The JSON scope containing the array to accumulate over.
 * @param func The binary function to apply for accumulation (e.g., addition for sum, multiplication for product).
 * @param initialValue The initial value for the accumulation (default is 0.0, but can be set to 1.0 for product or +-infinity for min/max).
 * @return The accumulated result, or NaN if the input is invalid (not a non-empty array of numeric values).
 */
template <typename F>
double accumulate(Data::JsonScope const& jsonDoc, F&& func, double const initialValue = 0.0) {
    static_assert(std::is_invocable_r_v<double, F, double, double>);

    if (jsonDoc.memberType(Base::TransformationModule::rootKey) != Data::KeyType::array) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double result = initialValue;
    std::size_t const size = jsonDoc.memberSize(Base::TransformationModule::rootKey);

    if (size == 0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    for (auto itemKey : Base::TransformationModule::rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(itemKey).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        result = std::invoke<F>(std::forward<F>(func), result, value); // NOLINT
    }
    return result;
}

} // anonymous namespace

void Statistics::bindTransformations() {
    bindTransformation(&sum, sumName, sumDesc);
    bindTransformation(&product, productName, productDesc);
    bindTransformation(&min, minName, minDesc);
    bindTransformation(&max, maxName, maxDesc);
    bindTransformation(&mean, meanName1, meanDesc);
    bindTransformation(&mean, meanName2, meanDesc);
    bindTransformation(&median, medianName, medianDesc);
    bindTransformation(&medianLower, medianLowerName, medianLowerDesc);
    bindTransformation(&medianUpper, medianUpperName, medianUpperDesc);
    bindTransformation(&stddev, stddevName, stddevDesc);
    bindTransformation(&range, rangeName, rangeDesc);
    bindTransformation(&mode, modeName, modeDesc);
}

bool Statistics::sum(Data::JsonScope& jsonDoc) {
    double const result = accumulate(jsonDoc, [](double const a, double const b) { return a + b; });
    if (std::isnan(result)) {
        return false;
    }
    jsonDoc.set(rootKey, result);
    return true;
}

bool Statistics::product(Data::JsonScope& jsonDoc) {
    double const result = accumulate(jsonDoc, [](double const a, double const b) { return a * b; }, 1.0);
    if (std::isnan(result)) {
        return false;
    }
    jsonDoc.set(rootKey, result);
    return true;
}

bool Statistics::min(Data::JsonScope& jsonDoc) {
    double const result = accumulate(jsonDoc, [](double const a, double const b) { return std::min(a, b); }, std::numeric_limits<double>::infinity());
    if (std::isnan(result)) {
        return false;
    }
    jsonDoc.set(rootKey, result);
    return true;
}

bool Statistics::max(Data::JsonScope& jsonDoc) {
    double const result = accumulate(jsonDoc, [](double const a, double const b) { return std::max(a, b); }, -std::numeric_limits<double>::infinity());
    if (std::isnan(result)) {
        return false;
    }
    jsonDoc.set(rootKey, result);
    return true;
}

bool Statistics::mean(Data::JsonScope& jsonDoc){
    double const result = accumulate(jsonDoc, [](double const a, double const b) { return a + b; });
    if (std::isnan(result)) {
        return false;
    }
    jsonDoc.set(rootKey, result / static_cast<double>(jsonDoc.memberSize(rootKey)));
    return true;
}

bool Statistics::median(Data::JsonScope& jsonDoc) {
    return calculateMedian(jsonDoc, MedianKind::conventional);
}

bool Statistics::medianLower(Data::JsonScope& jsonDoc) {
    return calculateMedian(jsonDoc, MedianKind::lower);
}

bool Statistics::medianUpper(Data::JsonScope& jsonDoc) {
    return calculateMedian(jsonDoc, MedianKind::upper);
}

bool Statistics::stddev(Data::JsonScope& jsonDoc) {
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }

    std::size_t const size = jsonDoc.memberSize(rootKey);
    if (size == 0) {
        return false;
    }

    std::vector<double> values;
    values.reserve(size);
    double sum = 0.0;
    for (auto const key : rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        values.push_back(value);
        sum += value;
    }

    double const mean = sum / static_cast<double>(size);
    double varianceSum = 0.0;
    for (double const& value : values) {
        varianceSum += (value - mean) * (value - mean);
    }
    double const stddevValue = std::sqrt(varianceSum / static_cast<double>(size));
    jsonDoc.set(rootKey, stddevValue);
    return true;
}

bool Statistics::range(Data::JsonScope& jsonDoc) {
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }

    std::size_t const size = jsonDoc.memberSize(rootKey);
    if (size == 0) {
        return false;
    }

    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -std::numeric_limits<double>::infinity();
    for (auto const key : rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
    }
    jsonDoc.set(rootKey, maximum - minimum);
    return true;
}

bool Statistics::mode(Data::JsonScope& jsonDoc) {
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }

    std::size_t const size = jsonDoc.memberSize(rootKey);
    if (size == 0) {
        return false;
    }

    std::map<double, std::size_t> frequencies;
    for (auto const key : rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        ++frequencies[value];
    }

    double modeValue = 0.0;
    std::size_t modeFrequency = 0;
    for (auto const key : rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(key).value();
        if (frequencies[value] > modeFrequency) {
            modeValue = value;
            modeFrequency = frequencies[value];
        }
    }
    jsonDoc.set(rootKey, modeValue);
    return true;
}

} // namespace Nebulite::Module::Transformation
