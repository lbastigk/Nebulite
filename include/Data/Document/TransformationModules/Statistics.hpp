/**
 * @file Statistics.hpp
 * @brief Class for statistics transformation modules.
 */

#ifndef NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_STATISTICS_HPP
#define NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_STATISTICS_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data::TransformationModules {

class Statistics final : public TransformationModule {
public:
    explicit Statistics(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override ;

    //------------------------------------------
    // Available Transformations

    // TODO: total, median, average, stddev, min, max, etc.
    //       Make sure to allow for non-numeric values if appropriate
};
} // namespace Nebulite::Data::TransformationModules
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_STATISTICS_HPP
