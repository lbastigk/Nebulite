/**
 * @file Statistics.hpp
 * @brief Class for statistics transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_STATISTICS_HPP
#define NEBULITE_TRANSFORMATION_MODULE_STATISTICS_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Statistics
 * @brief Statistical analysis transformations
 */
class Statistics final : public Data::TransformationModule {
public:
    explicit Statistics(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override ;

    //------------------------------------------
    // Available Transformations

    // TODO: Implement tests

    static bool sum(Data::JsonScope* scope);
    static auto constexpr sumName = "sum";
    static auto constexpr sumDesc = "Sums the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |sum -> {number}\n";

    static bool average(Data::JsonScope* scope);
    static auto constexpr averageName = "average";
    static auto constexpr averageDesc = "Calculates the average of the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |average -> {number}\n";

    static bool product(Data::JsonScope* scope);
    static auto constexpr productName = "product";
    static auto constexpr productDesc = "Multiplies the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |product -> {number}\n";

    static bool min(Data::JsonScope* scope);
    static auto constexpr minName = "min";
    static auto constexpr minDesc = "Finds the minimum value among the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |min -> {number}\n";

    static bool max(Data::JsonScope* scope);
    static auto constexpr maxName = "max";
    static auto constexpr maxDesc = "Finds the maximum value among the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |max -> {number}\n";

    static bool median(Data::JsonScope* scope);
    static auto constexpr medianName = "median";
    static auto constexpr medianDesc = "Calculates the median of the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |median -> {number}\n";

    static bool stddev(Data::JsonScope* scope);
    static auto constexpr stddevName = "stddev";
    static auto constexpr stddevDesc = "Calculates the standard deviation of the elements of the array in the current JSON value.\n"
        "Input must be an array of numbers. If any element is not a number, the current value is not an array or the array is empty, the transformation fails.\n"
        "Usage: |stddev -> {number}\n";

private:
    /**
     * @brief Helper function to accumulate values in an array using a provided (double, double) -> double function.
     * @param scope The JSON scope containing the array to accumulate over.
     * @param func The binary function to apply for accumulation (e.g., addition for sum, multiplication for product).
     * @param initialValue The initial value for the accumulation (default is 0.0, but can be set to 1.0 for product or +-infinity for min/max).
     * @return The accumulated result, or NaN if the input is invalid (not an array or contains non-numeric values).
     */
    static double accumulate(Data::JsonScope const* scope, std::function<double(double, double)> const& func, double const& initialValue = 0.0);
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_TRANSFORMATION_MODULE_STATISTICS_HPP
