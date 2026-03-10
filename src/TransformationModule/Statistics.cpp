#include "TransformationModule/Statistics.hpp"
#include "Data/Document/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void Statistics::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&sum, sumName, sumDesc);
    BIND_TRANSFORMATION_STATIC(&average, averageName, averageDesc);
    BIND_TRANSFORMATION_STATIC(&product, productName, productDesc);
    BIND_TRANSFORMATION_STATIC(&min, minName, minDesc);
    BIND_TRANSFORMATION_STATIC(&max, maxName, maxDesc);
    BIND_TRANSFORMATION_STATIC(&median, medianName, medianDesc);
    BIND_TRANSFORMATION_STATIC(&stddev, stddevName, stddevDesc);
}

double Statistics::accumulate(Data::JsonScope const* scope, std::function<double(double, double)> const& func, double const& initialValue) {
    if (scope->memberType(rootKey) != Data::KeyType::array) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double result = initialValue;
    size_t const size = scope->memberSize(rootKey);

    if (size == 0) {
        return result;
    }

    for (size_t i = 0; i < size; ++i) {
        auto const key = rootKey + "[" + std::to_string(i) + "]";
        double const value = scope->get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        result = func(result, value);
    }
    return result;
}


bool Statistics::sum(Data::JsonScope* scope) {
    double const result = accumulate(scope, [](double const a, double const b) { return a + b; });
    if (std::isnan(result)) {
            return false;
    }
    scope->set(rootKey, result);
    return true;
}

bool Statistics::average(Data::JsonScope* scope){
    double const result = accumulate(scope, [](double const a, double const b) { return a + b; });
    if (std::isnan(result)) {
            return false;
    }
    scope->set(rootKey, result / static_cast<double>(scope->memberSize(rootKey)));
    return true;
}

bool Statistics::product(Data::JsonScope* scope) {
    double const result = accumulate(scope, [](double const a, double const b) { return a * b; }, 1.0);
    if (std::isnan(result)) {
        return false;
    }
    scope->set(rootKey, result);
    return true;
}

bool Statistics::min(Data::JsonScope* scope) {
    double const result = accumulate(scope, [](double const a, double const b) { return std::min(a, b); }, std::numeric_limits<double>::infinity());
    if (std::isnan(result)) {
        return false;
    }
    scope->set(rootKey, result);
    return true;
}

bool Statistics::max(Data::JsonScope* scope) {
    double const result = accumulate(scope, [](double const a, double const b) { return std::max(a, b); }, -std::numeric_limits<double>::infinity());
    if (std::isnan(result)) {
        return false;
    }
    scope->set(rootKey, result);
    return true;
}

bool Statistics::median(Data::JsonScope* scope) {
    if (scope->memberType(rootKey) != Data::KeyType::array) {
        return false;
    }

    size_t const size = scope->memberSize(rootKey);
    if (size == 0) {
        return false;
    }

    std::vector<double> values(size);
    for (size_t i = 0; i < size; ++i) {
        auto const key = rootKey + "[" + std::to_string(i) + "]";
        double const value = scope->get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        values[i] = value;
    }

    std::ranges::sort(values.begin(), values.end());
    double medianValue;
    if (size % 2 == 0) {
        medianValue = (values[size / 2 - 1] + values[size / 2]) / 2.0;
    } else {
        medianValue = values[size / 2];
    }
    scope->set(rootKey, medianValue);
    return true;
}

bool Statistics::stddev(Data::JsonScope* scope) {
    if (scope->memberType(rootKey) != Data::KeyType::array) {
        return false;
    }

    size_t const size = scope->memberSize(rootKey);
    if (size == 0) {
        return false;
    }

    std::vector<double> values(size);
    double sum = 0.0;
    for (size_t i = 0; i < size; ++i) {
        auto const key = rootKey + "[" + std::to_string(i) + "]";
        double const value = scope->get<double>(key).value_or(std::numeric_limits<double>::quiet_NaN());
        if (std::isnan(value)) {
            return false;
        }
        values[i] = value;
        sum += value;
    }

    double const mean = sum / static_cast<double>(size);
    double varianceSum = 0.0;
    for (double const& value : values) {
        varianceSum += (value - mean) * (value - mean);
    }
    double const stddevValue = std::sqrt(varianceSum / static_cast<double>(size));
    scope->set(rootKey, stddevValue);
    return true;
}

} // namespace Nebulite::TransformationModule
