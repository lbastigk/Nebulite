/**
 * @file Statistics.hpp
 * @brief Class for statistics transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_STATISTICS_HPP
#define NEBULITE_TRANSFORMATION_MODULE_STATISTICS_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Statistics final : public Data::TransformationModule {
public:
    explicit Statistics(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override ;

    //------------------------------------------
    // Available Transformations

    // TODO: total, median, average, stddev, min, max, etc.
    //       Make sure to allow for non-numeric values if appropriate
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_STATISTICS_HPP
