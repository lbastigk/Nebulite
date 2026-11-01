#ifndef NEBULITE_MODULE_TRANSFORMATION_GENERATE_HPP
#define NEBULITE_MODULE_TRANSFORMATION_GENERATE_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>

// Nebulite
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Generate
 * @brief Transformations that generate new values.
 */
class Generate final : public Base::TransformationModule {
public:
    explicit Generate(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool iota(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr iotaName = "iota";
    static auto constexpr iotaDesc = "Generates a sequence of integer values.\n"
        "Usage: |iota <start> <end> -> {array}\n"
        "Generates an array containing the values from start (inclusive) to end (exclusive).\n"
        "If end is not greater than start, the result is an empty array.\n";

    static bool iotaMod(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr iotaModName = "iotaMod";
    static auto constexpr iotaModDesc = "Generates a sequence of modulo integer values.\n"
        "Usage: |iotaMod <mod> <start> <end> -> {array}\n"
        "Generates an array containing the values from start (inclusive) to end (exclusive), modulo the specified value.\n"
        "If end is not greater than start, the result is an empty array.\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_GENERATE_HPP
