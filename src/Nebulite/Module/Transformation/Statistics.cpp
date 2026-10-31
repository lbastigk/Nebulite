//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Transformation/Statistics.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Statistics::bindTransformations() {
    bindTransformation(&sum, sumName, sumDesc);
    bindTransformation(&product, productName, productDesc);
    bindTransformation(&min, minName, minDesc);
    bindTransformation(&max, maxName, maxDesc);
    bindTransformation(&mean, meanName1, meanDesc);
    bindTransformation(&mean, meanName2, meanDesc);
    bindTransformation(&median, medianName, medianDesc);
    bindTransformation(&stddev, stddevName, stddevDesc);
    bindTransformation(&range, rangeName, rangeDesc);
    bindTransformation(&mode, modeName, modeDesc);
}

double Statistics::accumulate(Data::JsonScope const& jsonDoc, std::function<double(double, double)> const& func, double const initialValue) {
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double result = initialValue;
    std::size_t const size = jsonDoc.memberSize(rootKey);

    if (size == 0) {
        return result;
    }

    for (auto itemKey : rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(itemKey).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        result = func(result, value);
    }
    return result;
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
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) {
        return false;
    }

    std::size_t const size = jsonDoc.memberSize(rootKey);
    if (size == 0) {
        return false;
    }

    std::vector<double> values;
    values.reserve(size);
    for (auto const key : rootKey.getArrayKeys(size)) {
        double const value = jsonDoc.get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        values.push_back(value);
    }

    std::ranges::sort(values.begin(), values.end());
    double medianValue = 0.0;
    if (size % 2 == 0) {
        medianValue = (values[size / 2 - 1] + values[size / 2]) / 2.0;
    } else {
        medianValue = values[size / 2];
    }
    jsonDoc.set(rootKey, medianValue);
    return true;
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
